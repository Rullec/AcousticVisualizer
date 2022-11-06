/*
 * =====================================================================================
 *
 *       Filename:  ModalModel.cpp
 *
 *        Version:  1.0
 *        Created:  11/30/10 17:51:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changxi Zheng (cz), cxzheng@cs.cornell.edu
 *                  Cornell University
 *
 * =====================================================================================
 */
#include "ModalModel.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>
#include "utils/term_msg.h"
#include "utils/macros.h"

#ifdef USE_MKL
#include <mkl.h>
#else
#error ERROR: We use cblas routines provided by MKL so far.
#endif

using namespace std;

const double CUTTING_FREQ = 16000.;
const double CUTTING_OMEGA = CUTTING_FREQ * 2. * M_PI;
// const double MIN_AUD_OMEGA = 20. * 2. * M_PI;

ModalModel::ModalModel(const std::string &modalFile, double density, double alpha, double beta) : density_(density), invDensity_(1. / density), alpha_(alpha), beta_(beta)
{
    if (density < 1E-8 || alpha < 0. || beta < 0.)
    {
        PRINT_ERROR("Invalid modal model parameters\n");
        SHOULD_NEVER_HAPPEN(-1);
    }

    PRINT_MSG("Load eigenmodes [%s] ...\n", modalFile.c_str());
    load_eigenmodes(modalFile.c_str());
}

/*!
 * Load eigen modes of rigid object from file
 */
void ModalModel::load_eigenmodes(const char *file)
{
    ifstream fin(file, ios::binary);

    fin.read((char *)&n3_, sizeof(int));       // size of eigen problem
    fin.read((char *)&numModes_, sizeof(int)); // number of modes
    if (fin.fail())
    {
        PRINT_ERROR("Cannot read file: %s\n", file);
        SHOULD_NEVER_HAPPEN(2);
    }

    eigenmodes_.resize(numModes_);
    PRINT_MSG("Load %d eigenmodes\n", numModes_);
    fin.read((char *)&eigenmodes_[0], sizeof(double) * numModes_);
    PRINT_MSG("Compute eigenmodes' frequencies ...\n");
    int nmds = 0;
    for (; nmds < numModes_; ++nmds)
    {
        // Here we divide by the density, because the eigenvalue analysis were done assuming a unit density
        eigenmodes_[nmds] *= invDensity_;
        if (eigenmodes_[nmds] > CUTTING_OMEGA * CUTTING_OMEGA)
            break;
    }
    PRINT_MSG("%d modes in audible range\n", nmds);
    numModes_ = nmds;

    // all the eigen vectors are stored in a n3 x nModes matrix
    // it is stored as v1 v2 ...
    eigenvec_.resize(n3_ * numModes_);
    fin.read((char *)&eigenvec_[0], sizeof(double) * eigenvec_.size());
    if (fin.fail())
    {
        PRINT_ERROR("Cannot read file: %s\n", file);
        SHOULD_NEVER_HAPPEN(2);
    }

    // compute modal parameters
    omega_.resize(numModes_);
    omegaD_.resize(numModes_);
    freqs_.resize(numModes_);
    c_.resize(numModes_);
    for (int i = 0; i < numModes_; ++i)
    {
        omega_[i] = sqrt(eigenmodes_[i]);
        if (std::isnan(omega_[i]) == true)
        {
            printf("[warn] mode %d eigenmode %.1f omega is nan, ignore\n", i, eigenmodes_[i]);
            continue;
        }
        freqs_[i] = omega_[i] * 0.5 * M_1_PI; // freq. = w / (2*pi)
        if (freqs_[i] < 20)
        {
            printf("[warn] mode %d eigenmode %.1f freq %.1f < 20, ignore\n", i, eigenmodes_[i], freqs_[i]);
            continue;
        }
        c_[i] = alpha_ * eigenmodes_[i] + beta_;
        double xi = c_[i] / (2. * omega_[i]);

        printf("[debug] mode %d eigenmodes %.1f omega %.1f freq %.1f alpha %.1f beta %.1e c %.1e xi %.1e\n", i, eigenmodes_[i], omega_[i], freqs_[i], alpha_, beta_, c_[i], xi);
        xi = std::min(1.0f - 1e-6, xi);
        if (xi >= 1.)
        {
            PRINT_ERROR("xi[%d] should always be in the range [0, 1]: %lf\n", i, xi);
            SHOULD_NEVER_HAPPEN(2);
        }
        omegaD_[i] = omega_[i] * sqrt(1. - xi * xi); // damped frequency
    }
}

/**
 * \brief           calculate modal force.
 * @param vid       afffected vertex id
 * @param imp       normal vector
 * @param out       输出向量. out += U^T * n / rho
 */
void ModalModel::accum_modal_impulse(int vid, const Vector3d *imp, double *out) const
{
    // out += U'*imp/rho
    cblas_dgemv(CblasColMajor, CblasTrans, 3, numModes_, invDensity_,
                &eigenvec_[vid * 3], n3_, (const double *)imp, 1, 1., out, 1);

    /*
        cblas_dgemv: 计算矩阵向量乘积
        @param Layout: 指出是Row Major还是ColMajor, = CblasColMajor
        @param trans: 指出运算类型,  = CblasTrans. 对CblasTrans来说, 它计算的是
            y := alpha*A'*x + beta*y;
        @param m: 指定矩阵A的行数 = 3
        @param n: 指定矩阵A的列数 = numModes_
        @param alpha: 标量alpha = invDensity_
        @param a: 数组, 尺寸为lda * k, 对CblasTrans来说, k是n, = eigenvec_[vid*3]
        @param lda: 指定a的首先维度; CblasColMajor时, lda最小值为max(1, m), = n3_
        @param x: 数组, 它的尺寸最小是(1+(m - 1)*abs(incx)), = imp
        @param incx: 表明x的每个elements占据的空间, 非0, = 1
        @param beta: 标量beta, = 1
        @param y: 数据, 尺寸至少为(1 +(n - 1)*abs(incy)) , = out
        @param incy: y数据大小, = 1

        当前:
        out := alpha *
    */
}
