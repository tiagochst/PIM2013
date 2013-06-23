#include "PointCloudGenerator.h"
#include "PointSet.h"
#include "PPMImage.h"
#include "Image.h"

PointSet* PointCloudGenerator::Generate (
    const PPMImage&         iTexture,
    Image&                  iDepth,
    std::vector<XnPoint3D>& iRealWorld,
    const std::string&      iPath
) { 
    const unsigned int& width = iTexture.Width ();
    const unsigned int& height = iTexture.Height ();
    const unsigned int& maxDepth = iDepth.GetMaxGreyLevel ();
    
    Image* tDepth = &iDepth;
    Image* fDepth = new Image ( width, height, maxDepth );
    unsigned int zeroCount = 0;
    do {
        zeroCount = 0;
        #pragma omp parallel for
        for ( int x = 0; x < width; x++ ) {
            for ( int y = 0; y < height; y++ ) {
                int value = tDepth->GetGreyLvl ( y, x );
    
                if ( value ) {
                    fDepth->SetGreyLvl(y,x,value);
                    continue;
                }
    
                float median = 0;
                unsigned int zeroes = 0;
                int sv[9];
                unsigned int count = 0;
                float sum = 0;
                for ( int dx = -1; dx <= 1; dx++ ) {
                    for ( int dy = -1; dy <= 1; dy++ ) {
    
                        int val = tDepth->GetGreyLvl ( y + dy, x + dx );
    
                        if ( !val ) continue;
    
                        //sum += val;
                        //count++;
                        if ( count ) {
                            sv[count++] = val;
                        } else {
                            int idx = 0;
                            for ( idx = 0; idx < count && sv[idx]<val; idx++ );
                            int tmp[9];
                            memcpy ( tmp, sv + idx, (count - idx)*sizeof(int));
                            sv[idx] = val;
                            memcpy ( sv + idx + 1, tmp, (count++ - idx)*sizeof(int));
                        }
                    }
                }
                if ( count && count & 0x1 ) {
                    median = sv[count >> 1];
                } else if ( count ) {
                    median = (sv[count>>1]+sv[count>>2+1]) / 2.0f;
                }
                if ( median == 0 ) zeroCount++;
                fDepth->SetGreyLvl ( y, x, nearbyint ( median ) );
                //if ( count ) sum /= (float)count;
                //else zeroCount++;
                //fDepth->SetGreyLvl ( y, x, nearbyint ( sum ) );
            }
        }
        if ( tDepth == &iDepth ) {
            tDepth = fDepth;
            fDepth = new Image ( width, height, maxDepth );
        } else {
            Image* tmp = fDepth;
            fDepth = tDepth;
            tDepth = tmp;
        }
    } while ( zeroCount );

    PointSet* ps = new PointSet ();
    //ps->Build ( iTexture, iDepth );
    ps->BuildFromKinectPointCloud ( iTexture, *fDepth, iRealWorld );

    fDepth->CreateAsciiPgm ( iPath + "depthMap.pgm" );

    delete fDepth;
    delete tDepth;

    return ps;
}
