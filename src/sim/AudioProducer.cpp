#include "AudioProducer.h"
#include <QtEndian>
#include <fstream>
#include "protobuf/sploosh.pb.h"
#include "utils/term_msg.h"
#include "utils/macros.h"
#include "transfer/FMMTransferEval.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

//#include <ctime>

#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

using namespace std;

static const int SR = 44100;
// static const double TS = 3;    // synthesize sound for at most 3 seconds
// static const double TS = 1;

void quickSort(float arr[], int index[], int left, int right)
{
    int i = left, j = right;
    float tmp;
    int tmp_i;
    float pivot = arr[(left + right) / 2];
    /* partition */
    while (i <= j)
    {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j)
        {
            tmp = arr[i];
            tmp_i = index[i];
            arr[i] = arr[j];
            index[i] = index[j];
            arr[j] = tmp;
            index[j] = tmp_i;
            i++;
            j--;
        }
    };
    /* recursion */
    if (left < j)
        quickSort(arr, index, left, j);
    if (i < right)
        quickSort(arr, index, i, right);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class WavPcmFile : public QFile
{
public:
    WavPcmFile(const QString &name, const QAudioFormat &format, QObject *parent = 0);
    bool open();
    void close();

private:
    void writeHeader();
    bool hasSupportedFormat();
    QAudioFormat format;
};

WavPcmFile::WavPcmFile(const QString &name, const QAudioFormat &format_, QObject *parent_)
    : QFile(name, parent_), format(format_)
{
}

bool WavPcmFile::hasSupportedFormat()
{
    return (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) || (format.sampleSize() > 8 && format.sampleType() == QAudioFormat::SignedInt && format.byteOrder() == QAudioFormat::LittleEndian);
}

bool WavPcmFile::open()
{
    if (!hasSupportedFormat())
    {
        setErrorString("Wav PCM supports only 8-bit unsigned samples "
                       "or 16-bit (or more) signed samples (in little endian)");
        printf("Failure1\n");
        return false;
    }
    else
    {
        if (!QFile::open(ReadWrite | Truncate))
        {
            printf("Failure2\n");
            return false;
        }
        writeHeader();
        return true;
    }
}

void WavPcmFile::writeHeader()
{
    QDataStream out(this);
    out.setByteOrder(QDataStream::LittleEndian);

    // RIFF chunk
    out.writeRawData("RIFF", 4);
    out << quint32(0); // Placeholder for the RIFF chunk size (filled by close())
    out.writeRawData("WAVE", 4);

    // Format description chunk
    out.writeRawData("fmt ", 4);
    out << quint32(16); // "fmt " chunk size (always 16 for PCM)
    out << quint16(1);  // data format (1 => PCM)
    out << quint16(format.channelCount());
    out << quint32(format.sampleRate());
    out << quint32(format.sampleRate() * format.channelCount() * format.sampleSize() / 8); // bytes per second
    out << quint16(format.channelCount() * format.sampleSize() / 8);                       // Block align
    out << quint16(format.sampleSize());                                                   // Significant Bits Per Sample

    // Data chunk
    out.writeRawData("data", 4);
    out << quint32(0); // Placeholder for the data chunk size (filled by close())

    Q_ASSERT(pos() == 44); // Must be 44 for WAV PCM
}

void WavPcmFile::close()
{
    // Fill the header size placeholders
    quint32 fileSize = size();

    QDataStream out(this);
    // Set the same ByteOrder like in writeHeader()
    out.setByteOrder(QDataStream::LittleEndian);
    // RIFF chunk size
    seek(4);
    out << quint32(fileSize - 8);

    // data chunk size
    seek(40);
    out << quint32(fileSize - 44);

    QFile::close();
}

inline bool exists_test(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

AudioProducer::AudioProducer(const QSettings &settings, const QDir &dataDir) : device_(NULL), normalizeScale_(-1.)
{
    ////// load data
    QString filename;
    // --- load vertex map ---
    {
        QString ff = settings.value("modal/vtx_map").toString();
        QFileInfo fInfo(ff);
        filename = fInfo.isRelative() ? dataDir.filePath(ff) : ff;
    }
    load_vertex_map(filename);

    // --- load modal shape matrix ---
    {
        QString ff = settings.value("modal/shape").toString();
        QFileInfo fInfo(ff);
        filename = fInfo.isRelative() ? dataDir.filePath(ff) : ff;
    }

    modal_ = new ModalModel(
        filename.toStdString(),
        settings.value("modal/density").toDouble(),
        settings.value("modal/alpha").toDouble(),
        settings.value("modal/beta").toDouble());

    for (int nt = 0; nt < NUM_THREADS; ++nt)
        mForce_[nt].resize(modal_->num_modes());

    // --- load moments ---
    {
        QString ff = settings.value("transfer/moments").toString();
        QFileInfo fInfo(ff);
        filename = fInfo.isRelative() ? dataDir.filePath(ff) : ff;
    }
    load_moments(filename);

    // get save .wav filename. if conflict, overwrite it.
    {
        QString ff = settings.value("gui/output_name").toString();
        QFileInfo fInfo(ff);
        outputWavFileName = fInfo.isRelative() ? dataDir.filePath(ff) : ff;
    }
    //////////////////////////////////////////////////////////////////////////////
    use_audio_device = settings.value("audio/use_audio_device").toBool();
    TS = settings.value("audio/TS").toDouble();
    //////////////////////////////////////////////////////////////////////////////
    // --- setup audio output device ---
    if (use_audio_device)
    {
        const QString devStr = settings.value("audio/device").toString();

        foreach (const QAudioDeviceInfo &deviceInfo,
                 QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        {
            if (devStr == deviceInfo.deviceName())
                device_ = new QAudioDeviceInfo(deviceInfo);
        }

        if (!device_)
        {
            SIM_ERROR("Cannot find the specified audio device\n");
            exit(-2);
        }
    }

    // --- allocate data buffer ---
    init();
}

AudioProducer::~AudioProducer()
{
    delete device_;
    if (use_audio_device)
        delete audioOutput_;
    /////////////////////////////////////////////////////////////////////////////
    delete modal_;

    for (size_t i = 0; i < transfer_.size(); ++i)
        delete transfer_[i];
}

void AudioProducer::load_moments(const QString &filename)
{
    sploosh::ModalMoments mms;
    ifstream fin(filename.toStdString().c_str(), ios::binary);
    if (!mms.ParseFromIstream(&fin))
    {
        SIM_ERROR("Cannot real protobuf file: %s\n", filename.toStdString().c_str());
        exit(-1);
    }

    const int nmms = (int)mms.moment_size();
    SIM_INFO("%d moments are detected\n", nmms);
    if (nmms < modal_->num_modes())
    {
        SIM_ERROR("Number of moments (%d) is smaller than the number of modes (%d)\n", nmms, modal_->num_modes());
        exit(-2);
    } // end if

    QFileInfo checkConfig(filename);
    QString dir = checkConfig.absoluteDir().absolutePath(); //.toStdString() << endl;
    transfer_.resize(modal_->num_modes());
    for (int mi = 0; mi < modal_->num_modes(); ++mi)
    {
        transfer_[mi] = new FMMTransferEval(mms.moment(mi), dir.toStdString());
    }
}

/*
 * Vertex map to map from surface vertex to tet vertex
 */
void AudioProducer::load_vertex_map(const QString &filename)
{
    int id1, id2;

    std::ifstream fin(filename.toStdString().c_str());
    if (fin.fail())
    {
        SIM_ERROR("Cannot read file: %s\n", filename.toStdString().c_str());
        exit(2);
    }
    fin >> numFixed_ >> id1; // # of fixed vertices in tet mesh
                             // & total number of surface vertices
    SIM_INFO("  # of fixed vertices: %d\n", numFixed_);
    vidS2T_.resize(id1);
    for (size_t i = vidS2T_.size(); i > 0; --i)
    {
        fin >> id1 >> id2;
        if (id2 >= vidS2T_.size())
        {
            SIM_ERROR("Id2 is out of range in geometry file\n");
            exit(3);
        }
        vidS2T_[id2] = id1;
    }
    if (fin.fail())
    {
        SIM_ERROR("Error occurred while reading file: %s\n", filename.toStdString().c_str());
        exit(2);
    }
    fin.close();
}

void AudioProducer::init()
{
    // setup audio format
    format_.setSampleRate(SR);
    // format_.setChannelCount( stereo_ ? 2 : 1 );
    format_.setChannelCount(1);
    format_.setSampleSize(16);
    format_.setCodec("audio/pcm");
    format_.setByteOrder(QAudioFormat::LittleEndian);
    format_.setSampleType(QAudioFormat::SignedInt);

    // now check the format
    if (use_audio_device)
    {
        if (!device_->isFormatSupported(format_))
        {
            SIM_ERROR("The specified format cannot be supported\n");
            exit(-2);
        }
    }

    // create buffer
    const qint64 len =
        (SR * TS * format_.channelCount() * format_.sampleSize() / 8);
    buffer_.resize(len);

    for (int nt = 0; nt < NUM_THREADS; ++nt)
        soundBuffer_[nt].resize(SR * TS * format_.channelCount());

    // create the IO device
    if (use_audio_device)
    {
        audioIO_.close();
        audioIO_.setBuffer(&buffer_);
        audioIO_.open(QIODevice::ReadOnly);

        audioOutput_ = new QAudioOutput(*device_, format_);
    }
}

void AudioProducer::play(const Tuple3ui &tri, const Vector3d &dir, const Point3d &cam, float amplitude)
{
    if (use_audio_device)
        audioIO_.close();

    // ====== fill the buffer ======
    //// for now, only do the single-channel synthesis
    single_channel_synthesis(tri, dir, cam, amplitude);
    /*
    unsigned char *ptr = reinterpret_cast<unsigned char *>(buffer_.data());
    const int channelBytes = format_.sampleSize() / 8;
    for(int ii = 0;ii < SR * TS; ++ ii)
    {
        const qreal x = sin( 2. * M_PI * 200. * static_cast<double>(ii) / static_cast<double>(SR) );
        const qint16 value = static_cast<qint16>(x * 32767);    // integer value

        qToLittleEndian<qint16>(value, ptr);
        ptr += channelBytes;
    }
    */

    if (use_audio_device)
    {
        audioIO_.open(QIODevice::ReadOnly);
        audioOutput_->reset();

        audioOutput_->start(&audioIO_);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    int seq = 1;
    string name = "audio1.wav";
    while (exists_test(name))
    {
        seq++;
        string name_seq = to_string(seq);
        name = "audio" + name_seq + ".wav";
    }

    QString wav_name = QString::fromStdString(name);

    WavPcmFile *m_file = new WavPcmFile(wav_name, format_);
    if (m_file->open())
    {
        // audioOutput_->start(m_file);
        m_file->write(buffer_);
        cout << name << " wrote\n";
    }

    // Stops the recording
    m_file->close();
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
}

/**
 * \brief   Calculate sound wave from collision and moments info
 *      该函数的外界调用是多线程的, index_t为线程id, 该函数内访问的共享资源只限于index_t所给定的部分
 */
void AudioProducer::single_channel_synthesis(const Tuple3ui &tri, const Vector3d &dir, const Point3d &cam, float amplitude, int index_t)
{
    // force in modal space: U^T f_cartesian
    // 清理 mForce_[index_t]. 需要指定f_caresian中非0项的id, 以及每个mode的特征向量的id对应项
    memset(mForce_[index_t].data(), 0, sizeof(double) * mForce_[index_t].size());
    // select a surface, then apply force to three vertices
    // force is double type
    // vidS2T is vertex map for surface VID to tet VID
    // tri is a surface ID
    // tri.x is a vertex ID
    modal_->accum_modal_impulse(vidS2T_[tri.x] - numFixed_, &dir, mForce_[index_t].data()); // 三角形顶点0
    modal_->accum_modal_impulse(vidS2T_[tri.y] - numFixed_, &dir, mForce_[index_t].data()); // 三角形顶点1
    modal_->accum_modal_impulse(vidS2T_[tri.z] - numFixed_, &dir, mForce_[index_t].data()); // 三角形顶点2

    // 此时, mForce_中存储着当前这个方向的力 dir / rho 时的模态力(广义力). mForce_ \in R^60
    const vector<double> &omegaD = modal_->damped_omega();
    const vector<double> &c = modal_->damping_vector();

    // multiply with the impulse response of each modes
    memset(soundBuffer_[index_t].data(), 0, sizeof(double) * soundBuffer_[index_t].size());
    const int totTicks = SR * TS;

    float sum_moments = 0;
    float sum_energy = 0; // 总能量
    float modes[modal_->num_modes()];
    int index[modal_->num_modes()];
    for (int i = 0; i < modal_->num_modes(); ++i)
    {
        sum_moments += abs(mForce_[index_t][i]);
        sum_energy += abs(mForce_[index_t][i]) * abs(mForce_[index_t][i]); // 力的平方就是总能量
        modes[i] = abs(mForce_[index_t][i]);                               // modes[i]存储了每个模态的激发程度
        index[i] = i;
    }
    quickSort(modes, index, 0, modal_->num_modes() - 1); // modes 按照激发程度, 从小到大排列. 并且记录排序后的index[]
    int num_modes = 0;

    // int seq=1;
    // string name="modes1_norm.txt";
    // while(exists_test(name)){
    //   seq++;
    //   string name_seq = to_string(seq);
    //   name = "modes_norm"+name_seq+".txt";
    // }
    // ofstream modesfile;
    // modesfile.open(name);

    float current_sum = 0;
    float current_energy = 0;

    // 对于每一个mode, 从小能量的Mode开始计算
    for (int j = 0; j < modal_->num_modes(); ++j)
    {
        // 获得其索引
        int i = index[modal_->num_modes() - 1 - j];
        if (current_energy <= sum_energy * 0.9) // 如果当前能量还不够90%, 则继续计算
        {
            if (omegaD[i] < 120)
            {
                printf("[debug] mode %d wd %.1f < 120, ignore\n", omegaD[i]);
                continue;
            }
            // 加入当前的能量
            current_energy += abs(mForce_[index_t][i]) * abs(mForce_[index_t][i]);
            FMMTransferEval::TComplex trans = transfer_[i]->eval(cam); // 对于这个mode, 评估其cam位置的声强传递函数

            num_modes += 1;
            // modesfile << abs(mForce_[i]) << "\n";
            /*
                SS = (U^T * f / rho)[i] * trans / wd * 0.4
            */
            const double SS = mForce_[index_t][i] * abs(trans) / omegaD[i] * 0.4;
            // cout<<"mode "<<i<<" mForce: "<<mForce_[i]<<"\n";
            // clock_t sub_start = clock();
            printf("[debug] mode %d w %.1f trans %.3f c %.3f\n", j, omegaD[i], abs(trans), c[i]);
            for (int ti = 0; ti < totTicks; ++ti)
            {
                const double ts = static_cast<double>(ti) / static_cast<double>(SR); // time
                const double amp = exp(-c[i] * 0.5 * ts);                            // exp(-xi * omega * t)
                /*
                    sound += e^{c_i * 0.5 * t} * SS * sin(wd * t) * amplitude
                                exp decay        unit_force_sound_pressure_this_mode * force * sin
                */
                if (amp < 1E-3)
                    break;
                double result = amp * SS * sin(omegaD[i] * ts) * amplitude; // sin(omega_d * t);
                soundBuffer_[index_t][ti] += result;
                if (std::isnan(result) == true)
                {
                    result = 0;
                    assert(false);
                }
            }
            // clock_t sub_end = clock();
            // long sub_elapsed_secs = long(sub_end - sub_start);
            // cout<<"sub time a click: "<<sub_elapsed_secs<<endl;
            //
        }
        // else {break;}
    } // end for

    // modesfile.close();

    // cout<<"sum of all moments: "<<sum_moments<<"\n";

    // cout<<"num moments: "<<num_modes<<"\n";

    // normalize the sound only for the first time, so the sound volume can change as camera moves
    // if ( normalizeScale_ < 0. )
    // {
    //     double AMP = 0;
    //     for(int ti = 0;ti < totTicks;++ ti)
    //         AMP = std::max( AMP, abs(soundBuffer_[index_t][ti]) );
    //     normalizeScale_ = 0.6 / AMP;
    // } // end if

    ///////////////////////////////////////////////////////////////////////////////
    // normalizeScale_ *= amplitude;
    ///////////////////////////////////////////////////////////////////////////////

    // unsigned char *ptr = reinterpret_cast<unsigned char *>(buffer_.data());
    // const int channelBytes = format_.sampleSize() / 8;
    // for(int ti = 0;ti < totTicks;++ ti)
    // {
    //     const qint16 value = static_cast<qint16>( soundBuffer_[ti] * normalizeScale_ * 32767 );
    //     qToLittleEndian<qint16>(value, ptr);
    //     ptr += channelBytes;
    // }
    // clock_t end = clock();
    // long elapsed_secs = long(end - begin);
    // cout<<"total time a click: "<<elapsed_secs<<endl;
}

void AudioProducer::generate_continuous_wav(QByteArray &buffer, vector<double> &whole_soundBuffer)
{
    //   int seq = 1;
    //   string name="continuous_audio1.wav";
    //   string name_raw="continuous_audio1.raw";
    //   while(exists_test(name) || exists_test(name_raw)){
    //     seq++;
    //     string name_seq = to_string(seq);
    //     name = "continuous_audio"+name_seq+".wav";
    //     name_raw = "continuous_audio"+name_seq+".raw";
    //   }

    QString wav_name = outputWavFileName;
    std::string name = wav_name.toStdString();

    // 写入wav
    WavPcmFile *m_file = new WavPcmFile(wav_name, format_);
    if (m_file->open())
    {
        // audioOutput_->start(m_file);
        m_file->write(buffer);
        printf("[debug] write to %s\n", name.c_str());
    }
    m_file->close();

    //   ofstream rawfile;
    //   rawfile.open(name_raw);
    //   for (int i = 0; i < whole_soundBuffer.size(); ++i){
    //     rawfile << whole_soundBuffer.at(i) << "\n";
    //   }
    //   cout<<name_raw<<" wrote\n";
    //   rawfile.close();
}
