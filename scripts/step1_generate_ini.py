import os
import os.path as osp
shapenet_dir = "/home/acoustic/Projects/AcousticVisualizer/ShapeNetData/" 
subdirs = ["20221115_full_sampling_13to20.send",  "20221115_full_sampling_21to30.send",  "20221115_full_sampling_41to50.send",  "20221115_full_sampling_51to54.send", "20221115_full_sampling_1to3.send",    "20221115_full_sampling_31to40.send",  "20221115_full_sampling_4to8.send",    "20221115_full_sampling_55to57.send"
]

import configparser
import json

def load_prop(prop_json):
    with open(prop_json, 'r') as f:
        cont = json.load(f)
        alpha = cont["alpha"]
        beta = cont["beta"]
        density = cont["density"]
    return alpha, beta, density

def generate_ini_inner(template_ini_path, surface_mesh_path, moments_path, shape_path, vtx_map_path, alpha, beta, density, target_ini_path):
    config = configparser.ConfigParser()
    config.optionxform = str
    # 1. [mesh]
    config.read(template_ini_path)
    config['mesh']['surface_mesh'] = surface_mesh_path
    config['mesh']['vertex_mapping'] = 'adddd'
    config['mesh']['sampling_info'] = ""

    config['audio']['use_audio_device'] = 'false'
    config['audio']['device'] = 'default'
    config['audio']['TS'] = '1.0'
    config['audio']['amplitude'] = '0.2'
    config['audio']['continuous'] = 'true'

    config['gui']['gui'] = 'false'
    config['gui']['output_name'] = ""

    config['transfer']['moments'] = moments_path

    config['modal']['shape'] = shape_path
    config['modal']['density'] = str(density)
    config['modal']['alpha'] = str(alpha)
    config['modal']['beta'] = str(beta)
    config['modal']['vtx_map'] = vtx_map_path

    config['camera']['x'] = "10"
    config['camera']['y'] = "10"
    config['camera']['z'] = "10"

    def repeat(ori, times):
        assert type(ori) == str
        return " ".join([ori for _ in range(times)])

    config['collisions']['time'] = repeat("0.0", 1)
    config['collisions']['ID'] = "0"
    config['collisions']['amplitude'] = repeat("1", 1)
    config['collisions']['norm1'] = repeat("1e-5", 1)
    config['collisions']['norm2'] = repeat("1", 1)
    config['collisions']['norm3'] = repeat("1e-5", 1)
    config['collisions']['camX'] = repeat(str(30), 1)
    config['collisions']['camY'] = repeat(str(30), 1)
    config['collisions']['camZ'] = repeat(str(30), 1)

    print(f"output to {target_ini_path}")
    with open(target_ini_path, 'w') as ff:
        config.write(ff)

template_ini_path = "/home/acoustic/Projects/AcousticVisualizer/scripts/template.ini"
from tqdm import tqdm
for subdir in subdirs:
    dir_path =  osp.join(shapenet_dir, subdir)
    for obj_subdir in  tqdm( os.listdir(dir_path)):
        final_path = osp.join(dir_path, obj_subdir)

        target_ini_path = osp.join(final_path, "generate0.ini")
        # if osp.exists(target_ini_path) == False:
        # begin to generate
        '''
        surface_mesh: from local dir
        moments: from local dir
        shape: from local dir
        vtx_map: local dir
        '''
        print(f"no ini in {target_ini_path}")
        surface_mesh_path = osp.join(final_path, "mesh.obj")
        moments_path = osp.join(final_path, "moments", "moments.pbuf")
        shape_path = osp.join(final_path, "mesh.ev")
        vtx_map_path  = osp.join(final_path, "mesh.vmap") 
        prop_path = osp.join(final_path, "prop.json")
        alpha, beta, density = load_prop(prop_path)

        generate_ini_inner(template_ini_path, surface_mesh_path, moments_path, shape_path, vtx_map_path, alpha, beta, density, target_ini_path)
        # else:
            # print(f"find ini in {target_ini_path}")