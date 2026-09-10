#include <Utility/CppMacros.h>
#include "WWMath/quat.cpp"

// Populate the reference implementation's tables using WWMath::Init's formulas,
// without starting its unrelated lookup-table manager or game allocation pools.
float _FastAcosTable[ARC_TABLE_SIZE];
float _FastAsinTable[ARC_TABLE_SIZE];
float _FastSinTable[SIN_TABLE_SIZE];
float _FastInvSinTable[SIN_TABLE_SIZE];

void Reference_Animation_Rotation(const float* first,const float* second,float weight,float* output) {
    static const bool initialized=[] {
        for(int i=0;i<ARC_TABLE_SIZE;++i) {
            const float value=float(i-ARC_TABLE_SIZE/2)*(1.0f/(ARC_TABLE_SIZE/2));
            _FastAcosTable[i]=acos(value);
        }
        for(int i=0;i<SIN_TABLE_SIZE;++i) {
            const float value=float(i)*2.0f*WWMATH_PI/SIN_TABLE_SIZE;
            _FastSinTable[i]=sin(value);
        }
        return true;
    }();
    (void)initialized;
    Quaternion a(first[0],first[1],first[2],first[3]);
    Quaternion b(second[0],second[1],second[2],second[3]),result;
    Fast_Slerp(result,a,b,weight);
    for(unsigned i=0;i<4;++i)output[i]=result[i];
}
