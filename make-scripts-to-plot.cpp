#include <iostream>
#include <sstream>
#include <fstream>

int MakeScriptsForAnimation(std::string str, int _DIM, int _L, int _PP_I)   {

    // Функция создания скриптов для анимации для плотности энергетических состояний

    // Функция создания скриптов для создания анимации для гистограммы

    std::stringstream ss;
    std::ofstream graph_sh;

    if(_DIM == 2)   {

    if(str == "G")  {
        
        for(int intervals = 0; intervals < _PP_I; intervals++)  {
    
            ss.str("");
            ss << "plot_test_g_graph-L=" << "2D_" << _L << "_PP=" << _PP_I << "-" << intervals << ".sh";
                
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-" << intervals << \
                            "/graphs/{1..20}.jpg animate-DoS-L=" << _L << "_PP=" << _PP_I <<"-" << intervals << ".gif\n";
            graph_sh.close();
    
        }
    
        ss.str("");
        ss << "plot_test_g_graph-L=" << "2D_" << _L << "_PP=" << _PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-AV" << \
                        "/graphs/{1..20}.jpg animate-DoS-L=" << _L << "_PP=" << _PP_I <<"-AV" << ".gif\n";
        graph_sh.close();
    }

    if(str == "H")  {

        for(int intervals = 0; intervals < _PP_I; intervals++)  {

            ss.str("");
            ss << "plot_test_hist_graph-L=" << "2D_"  << _L << "_PP=" << _PP_I << "-" << intervals << ".sh";
        
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << \
                    "/graphs/{1..200}.jpg animate-Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << ".gif\n";
            graph_sh.close();

        }

        ss.str("");
        ss << "plot_test_hist_graph-L=" << "2D_" << _L << "_PP=" << _PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-AV" << \
                            "/graphs/{1..200}.jpg animate-Hist-L=" << _L << "_PP=" << _PP_I <<"-AV" << ".gif\n";
        graph_sh.close();

    }

    }

    if(_DIM == 3)   {

    if(str == "G")  {
        
        for(int intervals = 0; intervals < _PP_I; intervals++)  {
    
            ss.str("");
            ss << "plot_test_g_graph-L=" << "3D_" << _L << "_PP=" << _PP_I << "-" << intervals << ".sh";
                
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-" << intervals << \
                            "/graphs/{1..20}.jpg animate-DoS-L=" << _L << "_PP=" << _PP_I <<"-" << intervals << ".gif\n";
            graph_sh.close();
    
        }
    
        ss.str("");
        ss << "plot_test_g_graph-L=" << "3D_" << _L << "_PP=" << _PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << _L << "_PP=" << _PP_I << "-AV" << \
                        "/graphs/{1..20}.jpg animate-DoS-L=" << _L << "_PP=" << _PP_I <<"-AV" << ".gif\n";
        graph_sh.close();
    }

    if(str == "H")  {

        for(int intervals = 0; intervals < _PP_I; intervals++)  {

            ss.str("");
            ss << "plot_test_hist_graph-L=" << "3D_" << _L << "_PP=" << _PP_I << "-" << intervals << ".sh";
        
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << \
                    "/graphs/{1..200}.jpg animate-Hist-L=" << _L << "_PP=" << _PP_I << "-" << intervals << ".gif\n";
            graph_sh.close();

        }

        ss.str("");
        ss << "plot_test_hist_graph-L=" << "3D_" << _L << "_PP=" << _PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << _L << "_PP=" << _PP_I << "-AV" << \
                            "/graphs/{1..200}.jpg animate-Hist-L=" << _L << "_PP=" << _PP_I <<"-AV" << ".gif\n";
        graph_sh.close();

    }

    }

    return 0;

}