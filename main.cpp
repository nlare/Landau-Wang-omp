#include <iostream>
#include <cstring>
#include <sstream>

#include "landau-wang-omp.h"
// #include "randomc.h"

int main(int argc, char *argv[])    {

    std::stringstream L_buf, PP_I_buf, MAX_MCS_COUNT_buf, DIM_buf;

    int L, PP_I, MAX_MCS_COUNT, DIM;
    
    if(argc < 3)    {

        std::cerr << "Set command line args!\n";

    }   else        {

        std::cout << "Exec with: ";

        for(int i = 0; i < argc; i++)   {

            if(strcmp(argv[i], "-D") == 0)  {

                if((i + 1) < argc)  {

                    std::cout << " " << argv[i] << " " << argv[i+1];
                    DIM_buf << argv[i+1];

                }

            }

            if(strcmp(argv[i], "-L") == 0)  {

                if((i + 1) < argc)  {

                    std::cout << " " << argv[i] << " " << argv[i+1];
                    L_buf << argv[i+1];

                }

            }

            if(strcmp(argv[i], "-PP") == 0)  {

                if((i + 1) < argc)  {

                    std::cout << " " << argv[i] << " " << argv[i+1];
                    PP_I_buf << argv[i+1];

                }

            }

            if(strcmp(argv[i], "-MCS") == 0)  {

                if((i + 1) < argc)  {

                    std::cout << " " << argv[i] << " " << argv[i+1];
                    MAX_MCS_COUNT_buf << argv[i+1];

                }

            }

        }

        std::cout << std::endl;

    }

    L = atoi(L_buf.str().c_str());
    DIM = atoi(DIM_buf.str().c_str());
    PP_I = atoi(PP_I_buf.str().c_str());
    MAX_MCS_COUNT = atoi(MAX_MCS_COUNT_buf.str().c_str());

    if(DIM == 3)    {
     
        if(L != 0 && PP_I >= 2 && ((PP_I % 2) == 0) && MAX_MCS_COUNT > 10000)    {

            LW3D(L, PP_I, MAX_MCS_COUNT);

            std::cout << "End of estimations with parameters: L = " << DIM << "D-" << L << ", PP_I = " << PP_I << ", MAX_MCS_COUNT = " << MAX_MCS_COUNT << std::endl;

        }   else    {

            std::cerr << "Set correct parameters! L must be > 0, PP_I > 1 and must be an even, MAX_MCS_COUNT > 10000" << std::endl;

        }

    }

    if(DIM == 2)    {

        if(L != 0 && PP_I >= 2 && ((PP_I % 2) == 0) && MAX_MCS_COUNT > 10000)    {

            LW2D(L, PP_I, MAX_MCS_COUNT);

            std::cout << "End of estimations with parameters: L = " << DIM << "D-" << L << ", PP_I = " << PP_I << ", MAX_MCS_COUNT = " << MAX_MCS_COUNT << std::endl;

        }   else    {

            std::cerr << "Set correct parameters! L must be > 0, PP_I > 1 and must be an even, MAX_MCS_COUNT > 10000" << std::endl;

        }

    }

    return 0;

}