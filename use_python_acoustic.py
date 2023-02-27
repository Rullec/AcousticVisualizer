import AcousticBinder

ini_path = "/home/acoustic/Projects/AcousticVisualizer/ShapeNetData/20221115_full_sampling_13to20.send/telephone_04401088_1b41282fb44f9bb28f6823689e03ea4_pvc/generate0.ini"
body = AcousticBinder.acoustic_body(0)
body.InitFromIni(ini_path, True)

ini_path = body.GetIniPath()
vpos = body.GetVertexPosVec()
triids = body.GetTriIdVec()
cam_pos = body.GetCamPos()
n_tris = int( len(triids)  / 3)
print(ini_path)
print(n_tris)

import time
for i in range(0, n_tris, 100):
    body.ClickTriangle(i)
    time.sleep(1)