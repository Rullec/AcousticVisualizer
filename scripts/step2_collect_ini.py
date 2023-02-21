import os
import os.path as osp
import json
shapenet_dir = "/home/acoustic/Projects/AcousticVisualizer/ShapeNetData/" 
subdirs = ["20221115_full_sampling_13to20.send",  "20221115_full_sampling_21to30.send",  "20221115_full_sampling_41to50.send",  "20221115_full_sampling_51to54.send", "20221115_full_sampling_1to3.send",    "20221115_full_sampling_31to40.send",  "20221115_full_sampling_4to8.send",    "20221115_full_sampling_55to57.send"
]

shapenet_new_surf = "/home/acoustic/Projects/ShapeNetV1/ShapeNetCore.v1"

def get_surface_mesh(type_id, obj_id):
    return osp.join(shapenet_new_surf, type_id, obj_id, "model.obj")
type_data = {}
for subdir in subdirs:
    now_dir = osp.join(shapenet_dir, subdir)
    for i in os.listdir(now_dir):
        splited = i.split("_")
        mat_str = splited[-1]
        obj_id = splited[-2]
        type_id = splited[-3]
        type_str = osp.join( * splited[:-3])
        
        if type_str not in type_data:
            type_data[type_str] = []
        
        surf_mesh = get_surface_mesh(type_id, obj_id)
        if osp.exists(surf_mesh): 
            cur_data =  {
                "type_str" : type_str,
                "type_id" : type_id,
                "obj_id" : obj_id,
                "ini": osp.join(now_dir, i, "generate0.ini"),
                "surface_mesh" : surf_mesh
                }

            type_data[type_str].append(cur_data)
        else:
            print(type_id)

for type_str in type_data:
    print(type_str, len(type_data[type_str]))

output = "shapenet_summary.json"
with open(output, 'w') as f:
    json.dump(type_data, f)
    print(f"output to {output}")