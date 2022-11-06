#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"
SIM_DECLARE_STRUCT_AND_PTR(tRay);

class cSimCallback
{
public:
    static void UpdateSimImGui();
    static void UpdateSimPerturbPos(const tVector4 &ori, const tVector4 &dir);
    static void CursorMove(int xpos, int ypos);
    static void CreatePerturb(tRayPtr ray);
    static void ReleasePerturb();
    static void MouseButton(int, int, int);
    static void Key(int a, int b, int c, int d);
};