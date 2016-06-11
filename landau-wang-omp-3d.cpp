/*
 * landau-wang.cpp
 *
 *  Created on: 24 мая 2014 г.
 *      Author: nlare
 * 
 *  Данная программа работает не непосредственно с плотностями энергетических состояний, а с энтропией состояний S(E) = ln(G(E))
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <omp.h>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "mersenne.cpp"

#define DEBUG
#define DEBUG_G
// #define DEBUG_H_SUM_G_SUM
// #define E_MIN_E_MAX_OUT
// #define COUT_IF_HIST_FLAT
// #define CHECK_H_DELT
// #define HIST_FLAT_OUT
#define COUT_EVERY_NUM_STEPS
// #define REPLICAEXHANGE 
#define energy(b) (2*(b)-2.0*(L*L))
#define PP_I 4
#define L 8
#define DISABLE_FLAT_CRITERIA
#define MAX_MCS_COUNT 100000

// int neighbour_spins(int,int);

int MakeScriptsForAnimation(std::string);

int main(int argc, char *argv[])  {

    int b, b_new, top_b;    // Текущий энергетический уровень и последующий, top_b - предельный энергетический уровень
    double f, f_min, ln_f;  /* "f" - начальный множитель для энергетических уровней, 
                             * "f_min" - минимальное значение множителя
                             * на каждый принятый шаг f_m = (f_m-1)^1/2
                             */
    int skip;                // Количество шагов с неизменной энергией
    int min_steps;
    double flat_threshold;   // Порог "плоскости" гистограммы
    double time_b, time_e;  
    double T;
    int it_count_g[PP_I],it_count_hist[PP_I];
    int it_count_av;
    int global_it_count;
    int count_mcs;
    double overlap;
    int h;
    int *E_min, *E_max;
    int *b_last;
    // int L = 16;

    // int count;
    double buffer; // Переменная для сохранения g(i) текущей реплики
    double exchange; // Переменная для сохранения g(i) реплики, с которой происходит обмен

    bool trigger = true;
    bool convergence_trigger[PP_I];
    int overlap_interval_begin, overlap_interval_end;

    std::cout << "adding g_averaged massive\n";
    double g_averaged[4*L*L*L];

    std::cout << "adding g_normalized massive\n";
    double g_normalized[4*L*L*L];

    std::cout << "adding hist_averaged massive\n";
    double hist_averaged[4*2*L*L*L];

    CRandomMersenne rg(13617235);

    srand(time(NULL));

    std::ofstream test_g_f, graph_g_f;
    std::stringstream ss;

    ss.str("");
    ss << "test_g";
    boost::filesystem::create_directories(ss.str().c_str());

    f = 2.7182818284;  // В работе Ландау-Ванга было указано значение "f" равное экспоненте 
    f_min = 1.000001;  // Данная переменная должна быть около единицы
    min_steps = 10000;
    skip = 10000;
    flat_threshold = 0.8;
    // overlap = 0.75;
    overlap = 0.95;
    h = 1;

    it_count_av = 0;
    count_mcs = 0;

    for(int rank = 0; rank < PP_I; rank++)  {

        it_count_g[rank] = 0;
        it_count_hist[rank] = 0;

    }  

    double max_in_pp_i, min_in_ge;

    struct _spins   {
    
        int spin[L][L][L];
        bool defect[L][L][L];

    }   system_of[PP_I];

    // top_b=L*L*L;

    struct _massive   {
        
        double g[4*L*L*L];
        int hist[4*L*L*L];

    }   massive[PP_I];

    for(int i = 0; i < 4*L*L*L; i++)    {
        g_averaged[i] = 0.0;
    }

    for(int i = 0; i < top_b; i++)  {
        g_averaged[i] = 0.0;
        hist_averaged[i] = 0.0;
    }

    for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
     
        for(int i = 0; i < L; i++)    {
            for(int j = 0; j < L; j++)    {
            	for(int k = 0; k < L; k++)	 {
                	system_of[pp_i].spin[i][j][k] = 1;
				}
            }
        }

        // for(int i = 0; i < L; i++)  {
        //     for(int j = 0;j < L; j++)   {
        //         if(rg.IRandom(0,10000.0)/10000.0 < 0.5)  
        //             system_of[pp_i].spin[i][j] = 1;
        //         else                    
        //             system_of[pp_i].spin[i][j] = -1;
        //     }
        // }

        for(int i = 0; i < 4*L*L*L; i++)    {
            massive[pp_i].g[i] = 1.0;
        }

        for(int i = 0; i < 4*L*L*L; i++)    {
            massive[pp_i].hist[i] = 1.0;
        }

        for(int i = 0; i < L; i++)  {
            for(int j = 0;j < L; j++)   {
            	for(int k = 0;k < L; k++)   {
                // if(rg.IRandom(0,10000.0)/10000.0 < 0.2) system_of[pp_i].defect[i][j] = true;
                	system_of[pp_i].defect[i][j][k] = false;
				}
            }
        }
    }

    if(L%2!=0) top_b=2*(L*L*(L-1))+1;
    else top_b=2*(L*L*L);

    E_min = new int [PP_I];
    E_max = new int [PP_I];

    b_last = new int [PP_I];

    double value = (top_b - top_b*overlap)/PP_I;

    for(int rank = 1; rank <= PP_I; rank++)   {

        E_min[rank-1] = value*(rank-1);
        E_max[rank-1] = top_b*overlap + value*rank;

        // if((rg.IRandom(0,1000)/1000) > 0.5) {

            if(E_min[rank-1]%2!=0)   {

                E_min[rank-1] +=1;

            }

            if(E_max[rank-1]%2!=0)  {

                E_max[rank-1] +=1;

            }

        std::cout << std::fixed << std::setprecision(4) << "E_min[" << rank-1 << "]=" << E_min[rank-1] \
                  << ", E_max[" << rank-1 << "]=" << E_max[rank-1] << ", overlap=" << overlap \
                  << std::endl;
    }    

    for(int i = 0; i < PP_I; i++)   {
        
        b_last[i] = E_min[i];  
        std::cout << "b_last[" << i << "] = " <<  b_last[i] << std::endl;

    } 

    omp_set_num_threads(PP_I);

    time_b = omp_get_wtime();

    std::cout << "Begin of estimation:" << std::endl;

    // Считаем пока "f" не примет минимальное значение
    while(f > f_min)    {

        #pragma omp parallel for \
        firstprivate(f,min_steps,skip,flat_threshold,h,E_min,E_max,top_b,it_count_g) \
        private(b_new,b,ln_f)  \
        shared(system_of,massive,b_last,buffer,convergence_trigger,overlap_interval_begin,overlap_interval_end)
        for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        
        int seed = 1 + rand() % 10000;
        CRandomMersenne Mersenne(seed);
        Mersenne.RandomInit(seed);

        int mcs;
        int count, n;
        int c = skip+1;
        double prob;    // Вероятность изменения энергетического уровня

        int L_C = L*overlap;

        if(L_C%2!=0) L_C+=1;

        // #pragma omp critical
        // std::cout << "L_C = " << L_C << std::endl;

        int global_it_count = 0;

        // int E_diff = E_max[pp_i] - E_min[pp_i];

        double g[top_b];

        ln_f = log(f);  // Для вычислений будем использовать логарифм множителя "f" 
        count = 1;  // Если гистограмма не стала плоской, останется равной единице и цикл продолжится.
        mcs = 0;
        n = 0;
        // b = 0;
        #pragma omp flush(b_last)
        // {
        b = b_last[pp_i];

        if(b == 0)    {
            b = E_min[pp_i];
        }

        int sum_spins = 0;

        std::cout << std::fixed << std::setprecision(8);

        // count_mcs = 0;

        // for(int i = 0; i < L; i++)    {
        //     for(int j = 0; j < L; j++)    {
        //         sum_spins += system_of[pp_i].spin[i][j];
        //     }
        // }

        // if(omp_get_thread_num()==0)
        // #pragma omp critical
        // std::cout << "b = " << b << "\t, sum_spins = " << sum_spins << "\t, rank = " << pp_i << std::endl;

        // }
        // convergence_trigger[pp_i] = false;
        // b = E_min[pp_i];

        // #pragma omp critical
        // std::cout << "chck = " << b << std::endl;

        // if(omp_get_thread_num() == 1)   {
        //     #pragma omp flush(massive,E_min,E_max)
        //     // massive[pp_i].g[20] = massive[0].g[20];

        //     // for(int i = E_min[pp_i]; i < E_max[0];i++)  {
        //         // massive[pp_i].g[i] = massive[0].g[i];
        //         // massive[pp_i].hist[i] = massive[0].hist[i];
        //     // }
        // }

        // #pragma omp flush(massive)
        for (int i = 0; i < top_b; i++)   {
            massive[pp_i].hist[i] = 0;
        }

        do {

        // if(mcs < 1000)   {
            // goto m3;
        // }

        for(int i = 0; i < L; i++)   {
            for(int j = 0; j < L; j++)  {
            	for(int k = 0; k < L; k++)  {
                m1:
                int ci = Mersenne.IRandom(0, L-1);
                int cj = Mersenne.IRandom(0, L-1);
                int ck = Mersenne.IRandom(0, L-1);

                if(system_of[pp_i].defect[ci][cj] == false) goto m1;  // Исключаем немагнитные спины

                int neighbour_spins = 0;
                #pragma omp flush(system_of)
                if(ci==0)   neighbour_spins=system_of[pp_i].spin[L-1][cj][ck];
                else        neighbour_spins=system_of[pp_i].spin[ci-1][cj][ck];
                if(ci==L-1) neighbour_spins+=system_of[pp_i].spin[0][cj][ck];
                else        neighbour_spins+=system_of[pp_i].spin[ci+1][cj][ck];
                if(cj==0)   neighbour_spins+=system_of[pp_i].spin[ci][L-1][ck];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj-1][ck];
                if(cj==L-1) neighbour_spins+=system_of[pp_i].spin[ci][0][ck];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj+1][ck];
                if(ck==0)   neighbour_spins+=system_of[pp_i].spin[ci][cj][L-1];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj][ck-1];
                if(ck==L-1) neighbour_spins+=system_of[pp_i].spin[ci][cj][0];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj][ck+1];

                // #pragma omp flush(system_of)
                b_new = b + system_of[pp_i].spin[ci][cj][ck]*neighbour_spins;   // Считаем новое состояние

                // std::cout << "b_new = " << b_new << std::endl;
                // #pragma omp critical
                // std::cout << "[" << omp_get_thread_num() << "]" << std::endl;
                // {
                    // if(omp_get_thread_num() == 1)
                    // std::cout << E_max[pp_i] << std::endl;
                    // std::cout << b_new << std::endl; 
                // }
                // #pragma omp flush
                // {

                #pragma omp flush(E_min, E_max)
                if((b_new < E_max[pp_i]) && (b_new > E_min[pp_i]) && system_of[pp_i].defect[i][j][k] == false)   {   // Если энергия в допустимых пределах

                    prob = exp(massive[pp_i].g[b]-massive[pp_i].g[b_new]);  // Вероятность принятия нового энергетического состояния
                    // prob = massive[pp_i].g[b]/massive[pp_i].g[b_new];
                    // #pragma omp critical
                    // if(omp_get_thread_num() == 1)
                    // std::cout << prob << "\n";
                    if(prob >= 1.0 || (double)(Mersenne.IRandom(1,10000)/10000.0) < prob)    {

                        b = b_new;
                        // #pragma omp flush(system_of)
                        // #pragma omp critical
                        system_of[pp_i].spin[ci][cj][ck] *= -1;

                    }
                    
                    massive[pp_i].g[b] += ln_f;   // Увеличиваем текущий энергетический уровень на логарифм "f"
                    // massive[pp_i].g[b]*=f;
                    massive[pp_i].hist[b] += 1;
                    // #pragma omp critical
                    // if(omp_get_thread_num() == 1)
                    // {
                    //     std::cout << "RUN IN ONE!" << std::endl;
                        // std::cout << "massive[" << pp_i << "].g[" << b << "]=" << massive[pp_i].g[b] << std::endl;
                    // }

                    // if(omp_get_thread_num()==0) {

                        // std::cout << "massive[0].g[8]=" << massive[0].g[8] << std::endl;

                    // }

                    n++;
                }
              }
           }
        }

        massive[pp_i].g[E_min[pp_i]+8] = massive[pp_i].g[E_min[pp_i]+6];

        // count++;

        #pragma omp critical 
        {
        // Если первая итерация и mcs делится на 100 без остатка, то 
        if(f == 2.7182818284 && mcs%100000 == 0)   {
        // if(omp_get_thread_num() == 0 && ln_f == 1)  {

                // std::cout << "IN!" << std::endl;
                count_mcs++;
                // std::cout << "C = " << count_mcs << std::endl;

                for(int pp_i = 0; pp_i < PP_I; pp_i++)  {

                // int first_not_null_g;
                // int number_of_iteration;
                // double min_g_value;
        
                it_count_hist[pp_i]++;

                ss.str("");
                ss << "test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << pp_i;
                boost::filesystem::create_directories(ss.str().c_str());

                ss << "/" << it_count_hist[pp_i] << ".dat";
                // std::cout << "write to " << ss.str() << std::endl;
    
                test_g_f.open(ss.str().c_str());
    
                for(int i = E_min[pp_i]; i < E_max[pp_i]; i++)  {
                    if((i!=2*E_max[pp_i]-1) && i%2==0)    {
                        // test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << massive[pp_i].g[i] << "\n";
                        test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << massive[pp_i].hist[i] << "\n";
                        // std::cout << "massive.g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
                    }
                }
    
                test_g_f.close();
    
                ss.str("");
                ss << "test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << pp_i << "/temp";
                boost::filesystem::create_directories(ss.str().c_str());
    
                ss.str("");
                ss << "test_g/Hist-" << "L=" << L << "_PP=" << PP_I << "-" << pp_i << "/temp/" << it_count_hist[pp_i] << ".plot";
                graph_g_f.open(ss.str().c_str());
    
                ss.str("");
                ss << "test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << pp_i << "/graphs";
                boost::filesystem::create_directories(ss.str().c_str());
    
                ss.str("");
                ss << "test_g/Hist-" << "L=" << L << "_PP=" << PP_I << "-" << pp_i << "/" << it_count_hist[pp_i] << ".dat";
    
                graph_g_f << "#!/usr/bin/gnuplot -persist\n" << \
                         "set terminal jpeg font arial 12 size 800,600\n" << \
                         "set output \"test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << pp_i << "/graphs/" << it_count_hist[pp_i] << ".jpg\"\n" << \
                         "set grid x y\n" << \
                         "set xtics 10 \n" << \
                         "set ytics 2000 \n" << \
                         "set mxtics 10 \n" << \
                         "set mytics 10  \n" << \
                         "set xrange [" << E_min[pp_i] << ":" << E_max[pp_i] << "]\n" << \
                         "set yrange [0:20000]\n" << \
                         "set xlabel \"i [" << E_min[pp_i] << ":" << E_max[pp_i] << "]" << "\"\n" << \
                         "set ylabel \"Hist(i)\"\n" << \
                         "plot \"" << ss.str() << "\" using 1:3 title \"landau-wang-" << L << "-iteration-" << it_count_hist[pp_i] << "\" with lines lt rgb \"red\"";
    
                graph_g_f.close();

            }

        // }

        }

        }

        // m3:

        mcs++;
        c++;

        // #pragma omp critical
        // {
        //     std::cout << "mcs[" << pp_i << "]:" << mcs << std::endl;
        // }

        #pragma omp flush(massive, E_min, E_max)
        // if(mcs == 5000 && pp_i == 0 && ln_f == 1)    {
        //     std::cout << "in cycle!\n";
        //     for(int i = E_min[pp_i]; i < E_max[pp_i]; i++)  {

        //         massive[pp_i+1].hist[i] = massive[pp_i].hist[i];

        //     }

        // }
        
        if((mcs >= min_steps) && (c >= skip))    {

            // int diff_between_g;
            
            c = 0;

        // #pragma omp critical
        // {
        //     std::cout << "flat_threshold = " << flat_threshold << std::endl; 
        // }
        
            #ifdef DEBUG
            // std::cout << "In count area. \n";
            // std::cout << "mcs = " << mcs << std::endl;
            #endif

            int num_of_exchange_replica;
            // int overlap_interval_begin, overlap_interval_end;
            int random_index_x, random_index_y;

            overlap_interval_begin = 0;
            overlap_interval_end = 0;

            #pragma omp critical
            {
            // Зададим граничные условия
            if(pp_i == 0)   {

                // std::cout << "\nIN SECTION!";
                num_of_exchange_replica = pp_i+1;
                // std::cout << "\nnum_of_exchange_replica = " << num_of_exchange_replica;

            }   else    {

                if(pp_i == PP_I-1)   {
                
                    num_of_exchange_replica = pp_i-1;

                }   else {

                    if(Mersenne.IRandomX(1,10000)/10000.0 > 0.5) num_of_exchange_replica = pp_i+1;
                    else num_of_exchange_replica = pp_i-1;

                }

            }

            if(pp_i < num_of_exchange_replica)  {

                overlap_interval_begin = E_min[num_of_exchange_replica];
                overlap_interval_end = E_max[pp_i];

            }   else    {
            
                overlap_interval_begin = E_min[pp_i];
                overlap_interval_end = E_max[num_of_exchange_replica];

            }

            }

            // std::cout << "overlap_interval_begin = " << overlap_interval_begin << "; overlap_interval_end = " << overlap_interval_end << std::endl;

            // #pragma omp flush(E_min)
            // diff_between_g = E_min[pp_i] - E_min[num_of_exchange_replica];
            // std::cout << "dbg = " << diff_between_g << std::endl;

            // overlap_interval_begin -= 2; 
            // overlap_interval_end -= 1; 

            // #ifdef DEBUG
            // if(pp_i == 1)   {
                // std::cout << pp_i << ":overlap_interval_begin=" << overlap_interval_begin << std::endl;
                // std::cout << pp_i << ":overlap_interval_end=" << overlap_interval_end << std::endl;
            // }
            // #endif

            // prob = 0.01;
            // count = 0;

            #pragma omp flush(E_min, E_max)

            // int first_not_null_g_for_pp_i, first_not_null_g_for_exchange_replica; 
            // double delta_G = 0.0;
            #ifdef REPLICAEXHANGE
            #pragma omp critical
            {           
                // for(int i = overlap_interval_begin; i < overlap_interval_end; i++)
                // for(int exchange_number = 0; exchange_number < (overlap_interval_end - overlap_interval_begin)/2; exchange_number++)
                {

                // for(int i = overlap_interval_begin; i < overlap_interval_end; i+=2)  {

                    // random_index_y = random_index_x - diff_between_g;

                    // if(random_index_x%2!=0)
                    // std::cout << "random_index_x = " << random_index_x << std::endl;
                    // std::cout << "random_index_y = " << random_index_y << std::endl;

                    #pragma omp flush(massive)
                    // if(convergence_trigger[num_of_exchange_replica]==false)
                    // if(massive[pp_i].hist[random_index_x] != 0 && massive[num_of_exchange_replica].hist[random_index_y] != 0)
                
                    // for(int exchange_number = 0; exchange_number < (overlap_interval_end - overlap_interval_begin)/2; exchange_number++)
                    {

                    // while(random_index_y%2!=0)  {
                    //     random_index_y = Mersenne.IRandom(0,top_b);
                    //     if(random_index_y%2==0) break;
                    // }

                    for(int exchange_number = 0; exchange_number < (overlap_interval_end - overlap_interval_begin)/2; exchange_number++)
                    {

                    random_index_x = Mersenne.IRandom(0,top_b);
                    // random_index_y = Mersenne.IRandom(0,top_b);
                    // random_index_y = random_index_x;

                    while(random_index_x%2!=0)  {
                        random_index_x = Mersenne.IRandom(0,top_b);
                        if(random_index_x%2==0) break;
                    }

                    random_index_y = random_index_x;

                    if((random_index_x > overlap_interval_begin+2) && (random_index_x < overlap_interval_end)\
                        && (random_index_y > overlap_interval_begin+2) && (random_index_y < overlap_interval_end))    {

                        // if(massive[pp_i].hist[random_index_x]!=0 && massive[pp_i].hist[random_index_x]!=0 \
                            // && massive[num_of_exchange_replica].hist[random_index_y]!=0 && massive[num_of_exchange_replica].hist[random_index_y]!=0)
                        #pragma omp flush(random_index_x,random_index_y)
                        // {
                            prob = (massive[pp_i].g[random_index_x]*massive[num_of_exchange_replica].g[random_index_y])/ \
                                   (massive[pp_i].g[random_index_y]*massive[num_of_exchange_replica].g[random_index_x]);


                        if(prob >= 1.0 || (double)(Mersenne.IRandom(1,10000)/10000.0) < prob) { //&& (fabs(massive[pp_i].g[random_index_y] - massive[num_of_exchange_replica].g[random_index_y])) < 100.0) {

                            // ---------------- EXHANGE BETWEEN SOME RANDOM INDEXES OF G[i] -----------------------

                            // if(abs(massive[pp_i].g[random_index_x] - massive[num_of_exchange_replica].g[random_index_y]) < 100.0)    {
                                
                                // std::cout << "init exchange between THREAD #" << pp_i << " and #" << num_of_exchange_replica << " : " << random_index_x << "[" << massive[pp_i].g[random_index_x] << "]" << " <-> " << random_index_y << "[" << massive[num_of_exchange_replica].g[random_index_y] << "]" << std::endl;
                                // #pragma omp flush(massive,buffer,random_index_x,random_index_y,ln_f)

                                double buffer = massive[pp_i].g[random_index_x];
                                massive[pp_i].g[random_index_x] = massive[num_of_exchange_replica].g[random_index_x];
                                massive[num_of_exchange_replica].g[random_index_x] = buffer;

                                buffer = massive[pp_i].g[random_index_y];
                                massive[pp_i].g[random_index_y] = massive[num_of_exchange_replica].g[random_index_y];
                                massive[num_of_exchange_replica].g[random_index_y] = buffer;

                                // if()

                                massive[pp_i].g[random_index_x] += ln_f;
                                massive[pp_i].hist[random_index_x] += 1;                                

                                massive[pp_i].g[random_index_y] += ln_f;
                                massive[pp_i].hist[random_index_y] += 1;                                

                                massive[num_of_exchange_replica].g[random_index_x] += ln_f;
                                massive[num_of_exchange_replica].hist[random_index_x] += 1;

                                massive[num_of_exchange_replica].g[random_index_y] += ln_f;
                                massive[num_of_exchange_replica].hist[random_index_y] += 1;
        
                        }   
                        
                    }

                    }
                    
                    }                    

                }
            }

            #endif

            int h_count = 0;
            double h_delt = 0.0;
            double h_sum = 0.0;

            h_delt = 0.0;

            // #pragma omp critical

            // #pragma omp critical
            {

            // Старый вариант проверки на плоскость гистограммы
            // ---------------------------------
            // for(int i = E_min[pp_i]; i < E_max[pp_i]; i++) {

            //     // if(massive[pp_i].hist[i] != 0)    {
            //     if(i%2 == 0)    {

            //         h_sum += (double)massive[pp_i].hist[i]/min_steps;
            //         h_count++;

            //     }

            // }

            // // #pragma omp flush
            // count = 0;  // Выходим из цикла по окончанию

            // for (int i = E_min[pp_i]; i < E_max[pp_i]; i++)    {
            //     if(massive[pp_i].hist[i] != 0)    {
            //         h_delt = (massive[pp_i].hist[i]/(h_sum/h_count))/min_steps;
            //         if((h_delt < flat_threshold) || (h_delt > 1+flat_threshold)) count =1;
            //         // else count = 0;

            //         // if(h_delt < flat_threshold) {
            //         //     std::cout << std::endl << "---------------------------" << std::endl;
            //         //     std::cout << i << ":" << h_delt << std::endl;
            //         //     std::cout << "---------------------------" << std::endl;
            //         // }

            //         // else goto m2;
            //     //     if((massive[pp_i].g[i] < E_max[pp_i]) && (massive[pp_i].g[i] > E_max[pp_i]-4 ) ) {
            //     //     std::cout << i << ":" << h_delt << std::endl;
            //     // }
            //     }
            // }

            // ---------------------------------

            // Новый вариант проверки на плоскость гистограммы основанный на среднеквадратичном отклонении
            // ---------------------------------
            #ifndef DISABLE_FLAT_CRITERIA
            {
                #pragma omp flush(massive)

                long double histav2 = 0.0; // Квадрат суммы
                long double hist2av = 0.0; // Сумма квадратов
                long double delta = 0.0;

                double min_hist = 0.0;
                int min_hist_index = 0;

                int hi_count = 0;

                count = 0;

                min_hist_index = (E_min[pp_i] + E_max[pp_i])/2;

                if(min_hist_index%2 != 0) min_hist_index += 1;

                min_hist = massive[pp_i].hist[min_hist_index];

                for(int i = E_min[pp_i]; i < E_max[pp_i]; i++)  {
                    
                    if(i%2 == 0 && (min_hist > massive[pp_i].hist[i]) && massive[pp_i].hist[i] > 0)    {

                        min_hist = massive[pp_i].hist[i];

                    }

                }

                #pragma omp flush(massive)
                // std::cout << "min_hist = " << min_hist << std::endl;

                for(int i = E_min[pp_i]; i < E_max[pp_i]; i++)  {

                    if((i > 4) && (i%2 == 0) && (massive[pp_i].hist[i] > 0))   {

                        // histav2 += (massive[pp_i].hist[i] - min_hist);
                        histav2 += (massive[pp_i].hist[i]);
                        // std::cout << pp_i << ": hist[" << i << "]=" << massive[pp_i].hist[i] - min_hist << std::endl;
                        // hist2av += (massive[pp_i].hist[i] - min_hist)*(massive[pp_i].hist[i] - min_hist);
                        hist2av += (massive[pp_i].hist[i])*(massive[pp_i].hist[i]);

                        hi_count++;

                    }

                }

                // hist2av = abs(hist2av);
                histav2 = histav2/hi_count;

                delta = hist2av - histav2*histav2;
                delta = powf(delta, 0.5);
                // delta = delta/min_steps;

                // if(delta > 1+flat_threshold || delta < 1-flat_threshold) count = 1;
                if(delta > flat_threshold) count = 1;
            }
            #endif

                if(mcs >= MAX_MCS_COUNT) count = 0;
                // else count = 1;
                #ifdef HIST_FLAT_OUT
                std::cout << "hist2av = " << hist2av << "; histav2 = " << histav2 << "; hi_count = " << hi_count << "; delta = " << delta << std::endl;
                #endif

            // ---------------------------------

            }

            // #pragma omp flush
            // if(count == 1 && h_delt > flat_threshold) goto m2;
            // printf("h_delt = %f\n",h_delt);
            #pragma omp critical
            {

            #ifdef CHECK_H_DELT
                std::cout << std::setprecision(8) << "f=" << f << ", ln_f=" << ln_f << ", flat_threshold = " << flat_threshold << ", h_delt = " << h_delt << ", count = " << count << ", PP = " << pp_i << std::endl;
            #endif

            #ifdef COUT_EVERY_NUM_STEPS
                std::cout << "L = " << L << ": f = " << f << ", ln_f = " << ln_f << "; PP = " << PP_I << "; thread = " << pp_i << ": mcs = " << mcs << std::endl; 
            #endif

            }
            // Убрать, если все будет хорошо считаться
            // if(mcs > 100000) goto m2;
        
        }   else count = 1;

        }   while(count);

        m2:

        #pragma omp critical

        #ifdef COUT_IF_HIST_FLAT
        std::cout << "Histogram is FLAT for relica #" << pp_i << std::endl;
        #endif

        if(omp_get_thread_num() == 0)    {

        for(int rank = 0; rank < PP_I; rank++)  {

        int first_not_null_g;
        double min_g_value;
        
        it_count_g[rank]++;
        // std::cout << "it[" << rank << "]=" << it_count_g[rank] << std::endl;

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << pp_i;
        boost::filesystem::create_directories(ss.str().c_str());

        ss << "/" << it_count_g[rank] << ".dat";
        // std::cout << "write to " << ss.str() << std::endl;

        test_g_f.open(ss.str().c_str());

        for(int i = E_min[rank]; i < E_max[rank]; i++)  {
            // if((i!=2*E_max[pp_i]-1) && (massive[pp_i].hist[i] != 0))    {
            if((i!=2*E_max[rank]-1) && (i%2 == 0))    {
            // if(((i!=2*E_max[pp_i]-1) && i%2 == 0) || i == 0)    {
                // test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << massive[pp_i].g[i] << "\n";
                test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << massive[rank].g[i] << "\n";
                // std::cout << "massive.g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
            }
        }

        test_g_f.close();

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << rank << "/temp";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "L=" << L << "_PP=" << PP_I << "-" << rank << "/temp/" << it_count_g[rank] << ".plot";
        graph_g_f.open(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << rank << "/graphs";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "L=" << L << "_PP=" << PP_I << "-" << rank << "/" << it_count_g[rank] << ".dat";

        graph_g_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << rank << "/graphs/" << it_count_g[rank] << ".jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xtics 20 \n" << \
                 "set ytics 1000 \n" << \
                 "set mxtics 5 \n" << \
                 "set mytics 5  \n" << \
                 "set xrange [" << E_min[rank] << ":" << E_max[rank] << "]\n" << \
                 "set xlabel \"i [" << E_min[rank] << ":" << E_max[rank] << "]" << "\"\n" << \
                 "set ylabel \"G(i)\"\n" << \
                 "plot \"" << ss.str() << "\" using 1:3 title \"landau-wang-" << L << "-iteration-" << it_count_g[rank] << "\" with lines lt rgb \"red\"";

        graph_g_f.close();

        }

        }

        b_last[pp_i] = b;
        // std::cout << "b_last[" << pp_i << "]= " << b_last[pp_i] << std::endl;
        // convergence_trigger[pp_i] = true;

        for(int i = 0; i < top_b; i++)  {

            massive[pp_i].hist[i] = 0.0;

        }

        // #pragma omp critical
        // std::cout << "-----------------------------\n";
        // std::cout << "#" << pp_i << ": mcs = " << mcs << " IS FLAT" << std::endl; 
        // std::cout << "-----------------------------\n";

        }

        // #pragma omp barrier

        // Процедура усреднения и перераспределения между репликами среднего значения плотности энергетических состояний

        #pragma omp flush(massive,hist_averaged,g_averaged)

        if(omp_get_thread_num() == 0)   {

        int div_averaging[top_b];

        // for(int i = 0; i < overlap_interval_begin+4; i++) {
        //     div_averaging[i] = 1;
        //     // g_averaged[i] -= 1;
        // }

        // for(int i = overlap_interval_begin+4; i < overlap_interval_end; i++) {
        //     div_averaging[i] = 2;
        //     // g_averaged[i] -= 1;
        // }

        // for(int i = overlap_interval_end; i < top_b; i++) {
        //     div_averaging[i] = 1;
        //     // g_averaged[i] -= 1;
        // }

        // std::cout << "TEST1!\n";
        // for(int )

        for(int i = 0; i < top_b; i++)  {
            if(PP_I == 2) div_averaging[i] = PP_I;
            if(PP_I >= 4) div_averaging[i] = 0;
        }

        div_averaging[0] = 1;
        div_averaging[2] = 1;
        div_averaging[4] = 1;

        for(int i = 0; i < 2*top_b; i++)    {
            g_averaged[i] = 1.0;
            hist_averaged[i] = 0.0;
        }

        // std::cout << "TEST2!\n";

        // std::cout << "overlap_interval_begin = " << overlap_interval_begin << "; overlap_interval_end = " << overlap_interval_end << std::endl;
        #pragma omp flush(massive, E_min, E_max)

        for(int i = 0; i < top_b; i++)  {

            #ifdef DEBUG_H_SUM_G_SUM
            std::cout << "------------------------------------------------------------------" << std::endl;
            #endif

            max_in_pp_i = 0;   
            for(int rank = 0; rank < PP_I; rank++)  {
                 // if(massive[pp_i].hist[i]!=0) 
                 if(i%2 == 0 || i == 0)  
                 {
                    // -------------- CHOOSE MAX G[E] IN ALL REPLICAS -----------------
                    // if(massive[pp_i].g[i] > max_in_pp_i)   {    
                    //     max_in_pp_i = massive[pp_i].g[i];
                    //     g_averaged[i] = max_in_pp_i;
                    // }
                    // -------------- CHOOSE MAX G[E] IN ALL REPLICAS -----------------
    
                    // -------------- AVERAGING ALL REPLICAS G[E] -----------------

                    hist_averaged[i] = hist_averaged[i] + massive[rank].hist[i];
                    // std::cout << "----------------------------------------------" << std::endl;

                    // hist_averaged[i] = 1.0;

                    // g_averaged[i] += massive[pp_i].g[i];

                    /*
                     * Only for L = 8
                     * 
                     */
                    if(PP_I == 2)   {
                        
                        if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                            div_averaging[i]++;

                            // g_averaged[i] = g_averaged[i] + massive[rank].g[i];

                        }

                        if(rank == 0)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 1)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                    }

                    if(PP_I == 4)   {
                        
                        if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                            div_averaging[i]++;

                            // g_averaged[i] = g_averaged[i] + massive[rank].g[i];

                        }

                        if(rank == 0)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 1)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 2)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 3)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                    }

                    if(PP_I == 8)   {
                        
                        if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                            div_averaging[i]++;

                            // g_averaged[i] = g_averaged[i] + massive[rank].g[i];

                        }

                        if(rank == 0)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 1)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 2)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 3)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 4)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 5)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 6)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 7)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }                                                                        

                    }

                    if(PP_I == 16)   {
                        
                        if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                            div_averaging[i]++;

                            // g_averaged[i] = g_averaged[i] + massive[rank].g[i];

                        }

                        if(rank == 0)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 1)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 2)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 3)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 4)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 5)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 6)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 7)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }    

                        if(rank == 8)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 9)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 10)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 11)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 12)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 13)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 14)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }

                        if(rank == 15)   {

                            if((i > E_min[rank]+4) && (i < E_max[rank]))    {

                                g_averaged[i] = g_averaged[i] + massive[rank].g[i]; 

                            }

                        }


                    }


                    // g_averaged[i] = g_averaged[i] + massive[rank].g[i];


                    // -------------- AVERAGING ALL REPLICAS G[E] ----------------
                    // if(f == 2.7182818284)    

                    // if(i > overlap_interval_begin && i < overlap_interval_end)  {


                    //     // ...
                    // }   else    {

                    //     // g_averaged[i] -= 1;
                    //     if(PP_I == 2) div_averaging[i] = 1;

                    // }
                }

                #ifdef DEBUG_H_SUM_G_SUM
                std::cout << "pp_i = " << rank << ": i: " << i << ": H_SUM=" << hist_averaged[i] << ", H[" << rank << "]=" << (double) massive[rank].hist[i] \
                              << ", G_SUM=" << g_averaged[i] << ", G[" << rank << "]=" << massive[rank].g[i] << std::endl;
                #endif

                #ifdef E_MIN_E_MAX_OUT
                std::cout << "E_min[" << 0 << "]=" << E_min[0] << std::endl; 
                std::cout << "E_min[" << 1 << "]=" << E_min[1] << std::endl; 
                std::cout << "E_max[" << 0 << "]=" << E_max[0] << std::endl; 
                std::cout << "E_max[" << 1 << "]=" << E_max[1] << std::endl; 
                #endif
            }   
        }

        std::cout << std::endl;

        int count = 0;

        #pragma omp flush(massive)

        // if(omp_get_thread_num() == 0)   {
            // std::cout << "Check1: g_averaged = " << g_averaged[0] << std::endl;
        // }

        for(int rank = 0; rank < PP_I; rank++)     {

        #ifdef E_MIN_E_MAX_OUT  
        {
            std::cout << "E_min[" << rank << "]=" << E_min[rank] << "; E_max[" << rank << "]=" << E_max[rank] << std::endl;
            std::cout << std::endl;
        }
        #endif

        }

        for(int i = 0; i < top_b; i++)     {

            if(i%2 == 0 || i == 0)  {

                count++;
                g_averaged[i]/=div_averaging[i];
                hist_averaged[i]/=div_averaging[i];

                // #define DEBUG_G
                #ifdef DEBUG_G
                if(PP_I >= 4)   {

                    std::cout << "G0[" << i << "]=" << massive[0].g[i] \
                          << " :: G1[" << i << "]=" << massive[1].g[i] << ":: G2[" << i << "]=" << massive[2].g[i] \
                          << ":: G3[" << i << "]=" << massive[3].g[i] << ":: G_AV[" << i << "]=" << g_averaged[i] \
                          << " :: DIV:" << div_averaging[i] << " :: Count = " << count << std::endl;

                }

                if(PP_I == 2)   {

                    std::cout << "G0[" << i << "]=" << massive[0].g[i] \
                          << " :: G1[" << i << "]=" << massive[1].g[i] \
                          << ":: G_AV[" << i << "]=" << g_averaged[i]  \
                          << " :: DIV:" << div_averaging[i] << " :: Count = " << count << std::endl;                    

                }
                #endif 

            }
        }

        if(f == 2.7182818284)    {

        // for(int pp_i = 0; pp_i < PP_I; pp_i++)  {

        int first_not_null_g;
        double min_g_value;
        
        it_count_av++;

        ss.str("");
        ss << "test_g/DoS-L=" << "3D_" << L << "_PP=" << PP_I << "-AV";
        boost::filesystem::create_directories(ss.str().c_str());

        ss << "/" << it_count_av << ".dat";
        // std::cout << "write to " << ss.str() << std::endl;

        test_g_f.open(ss.str().c_str());

        for(int i = 0; i < top_b; i++)  {
            // if((i!=2*top_b-1) && (hist_averaged[i] != 0))    {
            if(((i!=2*top_b-1) && (i%2 == 0)) || i == 0)    {
                test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << g_averaged[i] << "\n";
                // std::cout << "massive.g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
            }
        }

        test_g_f.close();

        ss.str("");
        ss << "test_g/DoS-L=" << "3D_" << L << "_PP=" << PP_I << "-AV" << "/temp";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "3D_" << "L=" << L << "_PP=" << PP_I << "-AV" << "/temp/" << it_count_av << ".plot";
        graph_g_f.open(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-L=" << "3D_" << L << "_PP=" << PP_I << "-AV" << "/graphs";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "3D_" << "L=" << L << "_PP=" << PP_I << "-AV" << "/" << it_count_av << ".dat";

        graph_g_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"test_g/DoS-L=" << "3D_" << L << "_PP=" << PP_I << "-AV" << "/graphs/" << it_count_av << ".jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"i\"\n" << \
                 "set ylabel \"G(i)\"\n" << \
                 "plot \"" << ss.str() << "\" using 1:3 title \"landau-wang-" << L << "-iteration-" << it_count_av << "\" with lines lt rgb \"red\"";

        graph_g_f.close();

        // }

        }

        // std::cout << "Check2" << std::endl;   

        #pragma omp flush(massive, g_averaged)

        for(int rank = 0; rank < PP_I; rank++)  {
            for(int i = 0; i < top_b; i++)  {
    
                massive[rank].g[i] = g_averaged[i];
                massive[rank].hist[i] = 0;
                hist_averaged[i] = 0;

                // if(i%2==0) massive[rank].hist[i] = 1.0;
    
            }
        }

        }

        #pragma omp barrier
        
        // if(omp_get_thread_num() == 0)   {
            // massive[0].g[0] = 0.0;
            // g_averaged[0] = 0.0;
        // }
        
        #pragma omp flush(massive)

        f = pow(f, 0.5);    // Изменяем множитель

    }   // while (f > f_min) cycle 

    std::cout << "Ending estimation of G." << std::endl;
    std::cout << "Begin Normalising of G:" << std::endl;

    // std::cout << "Thread #" << omp_get_thread_num() << std::endl;

    #pragma omp flush(massive)

    min_in_ge = g_averaged[6]; // index may be anything (not L*L)

    for(int i = 0; i < top_b; i++)  {
        // if(hist_averaged[i]!=0)  {
        if(i%2 == 0 && i > 4 && g_averaged[i] != 1.0)  {
            if(g_averaged[i] < min_in_ge)
            min_in_ge = g_averaged[i];
        }
    }

    // g_averaged[top_b] = min_in_ge;
    g_averaged[top_b] = g_averaged[top_b-2];

    if(PP_I == 2)    {

        massive[0].g[top_b] = g_averaged[top_b];
        massive[1].g[top_b] = g_averaged[top_b];

    }

    if(PP_I == 4)    {

        massive[0].g[top_b] = g_averaged[top_b];
        massive[1].g[top_b] = g_averaged[top_b];
        massive[2].g[top_b] = g_averaged[top_b];
        massive[3].g[top_b] = g_averaged[top_b];

    }

    if(PP_I == 8)    {

        massive[0].g[top_b] = g_averaged[top_b];
        massive[1].g[top_b] = g_averaged[top_b];
        massive[2].g[top_b] = g_averaged[top_b];
        massive[3].g[top_b] = g_averaged[top_b];
        massive[4].g[top_b] = g_averaged[top_b];
        massive[5].g[top_b] = g_averaged[top_b];
        massive[6].g[top_b] = g_averaged[top_b];
        massive[7].g[top_b] = g_averaged[top_b];

    }

    if(PP_I == 16)    {

        massive[0].g[top_b] = g_averaged[top_b];
        massive[1].g[top_b] = g_averaged[top_b];
        massive[2].g[top_b] = g_averaged[top_b];
        massive[3].g[top_b] = g_averaged[top_b];
        massive[4].g[top_b] = g_averaged[top_b];
        massive[5].g[top_b] = g_averaged[top_b];
        massive[6].g[top_b] = g_averaged[top_b];
        massive[7].g[top_b] = g_averaged[top_b];
        massive[8].g[top_b] = g_averaged[top_b];
        massive[9].g[top_b] = g_averaged[top_b];
        massive[10].g[top_b] = g_averaged[top_b];
        massive[11].g[top_b] = g_averaged[top_b];
        massive[12].g[top_b] = g_averaged[top_b];
        massive[13].g[top_b] = g_averaged[top_b];
        massive[14].g[top_b] = g_averaged[top_b];
        massive[15].g[top_b] = g_averaged[top_b];

    }

    std::cout << "min_in_g[E] = " << min_in_ge << std::endl;

    // -------------- FOR READISTRIBUTED G[E] -----------------
    // for(int i = 0; i < top_b; i++)  {
    //     if(hist_averaged[i]!=0)
    //     g_normalized[i] = g_averaged[i] - min_in_ge;
    // }
    // -------------- FOR READISTRIBUTED G[E] -----------------

    // -------------- FOR AVERAGED G[E] -----------------    
    // for(int i = 0; i < top_b; i++)  {
        // if(hist_averaged[i]!=0)
    // }
    // -------------- FOR AVERAGED G[E] ----------------- 



    for(int i = 0; i <= top_b; i++)  {
        
        if(i%2 == 0 && i > 4)   {
            g_normalized[i] = g_averaged[i] - min_in_ge;
        }
        // g_averaged[i]/=div_averaging[i];
        // if(g_averaged[i] > 1.0)
        // if(L == 8)  {
            // if(massive[0].hist[i]!=0 || massive[ 1].hist[i]!=0)
            if(i%2 == 0 && i > 4)   {
            std::cout << std::fixed << "g[" << i << "]=" << g_averaged[i] \
                      << ":\t" << massive[0].g[i] << "\t" << massive[1].g[i] << "\t" \
                      << g_normalized[i] << std::endl;
                  // << "\t" << massive[0].g[i] - massive[0].g[4] << "\t" \
                  // << massive[1].g[i] - massive[1].g[20] << "\t" \
    
            }
        // }

    }  

    std::cout << "End Averaging of G." << std::endl;

    MakeScriptsForAnimation("G"); // Для G[]
    MakeScriptsForAnimation("H"); // Для Hist[]

    double EE, EE2, GE, Ut, Ft, St, Ct, lambdatemp, lambda;

    std::ofstream out_f_td, out_f_ds, plot_f, script_f, time_f;

    char * filename_out_td = new char [100];
    char * filename_out_ds = new char [100];

    ss.str("");
    ss << "results/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "_MAXMCS=" << MAX_MCS_COUNT << ".dat";

    strcpy(filename_out_td ,ss.str().c_str());

    out_f_td.open(filename_out_td);
    if(!out_f_td) std::cout << "Cannot open " << filename_out_td << ".Check permissions or free space";
    out_f_td << "T\tUt\tFt\tSt\tCt\n";

    ss.str("");
    ss << "results/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I << "_MAXMCS=" << MAX_MCS_COUNT << ".dat";

    strcpy(filename_out_ds, ss.str().c_str());

    out_f_ds.open(filename_out_ds, std::ios::out);
    if(!out_f_ds) std::cout << "Cannot open " << filename_out_ds << ".Check permissions or free space";
    out_f_ds << "i\tE(i)\tg[i]\n";

    std::cout << "Begin estimation for T..." << std::endl; 
    // TEST
    // for(int i = 0; i < top_b; i++)  {

    //     std::cout << i << ":" << g_averaged[i] << std::endl;

    // }

    for(double T = 0.01; T <= 8; T += 0.01)  {

        EE = 0;
        EE2 = 0;
        GE = 0;

        lambda = 0;
        lambdatemp = 0;

        for(int i = 0; i < top_b; i++)  {
            // if((i!=0) && i%2 == 0 && (i!=L*L-1) && g_averaged[i]==g_averaged[i])    {
            if(i%2 == 0 && (i!=L*L-1))    {
                lambdatemp = g_normalized[i] - energy(i)/T;
                if(lambdatemp > lambda) lambda = lambdatemp;
            }
        }

        for(int i = 0; i < top_b; i++) {
            if(i != 1 && i%2 == 0 && (i!=L*L-1))    {
                EE += energy(i)*exp(g_normalized[i]-(energy(i))/T-lambda);
                EE2 += energy(i)*energy(i)*exp(g_normalized[i]-(energy(i))/T-lambda);
                GE += exp(g_normalized[i]-energy(i)/T-lambda);
            }
        }

        Ut = EE/GE;
        Ft = -T*lambda-(T)*log(GE);
        St = (Ut-Ft)/T;
        Ct = ((EE2/GE)-(Ut*Ut))/(T*T);

        // if((Ut==Ut)==true&&(Ft==Ft)==true&&(St==St)==true&&(Ct==Ct)==true) // Не пишем NaN 
        out_f_td << std::fixed << std::setprecision(8) << T << "\t" << Ut/(L*L) << "\t" << Ft/(L*L) << "\t" << St/(L*L) << "\t" << Ct/(L*L) << "\n";

    }

    for(int i = 0; i <= top_b; i++)  {
        if((i!=2*(L*L)-1) && i%2==0 && i > 4)    {
            if((g_normalized[i]) >= 0 && (g_normalized[i])==(g_normalized[i]))
            out_f_ds << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << g_normalized[i] << "\n";
        }
    }

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());
    
    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "/Ut.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ut.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"T\"\n" << \
                 "set ylabel \"Ut\"\n" << \
                 "plot \"" << filename_out_td << "\" using 1:2 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "/Ft.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ft.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"T\"\n" << \
                 "set ylabel \"Ft\"\n" << \
                 "plot \"" << filename_out_td << "\" using 1:3 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "/St.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/TermodinamicalStat_L=" << L << "_PP=" << PP_I <<  "/St.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"T\"\n" << \
                 "set ylabel \"St\"\n" << \
                 "plot \"" << filename_out_td << "\" using 1:4 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "/Ct.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ct.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"T\"\n" << \
                 "set ylabel \"Ct\"\n" << \
                 "plot \"" << filename_out_td << "\" using 1:5 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();

    ss.str("");
    ss << "temp/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "temp/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I << "/Ei.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/DensityStat_L=" << L << "_PP=" << PP_I << "/Ei.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"i\"\n" << \
                 "set ylabel \"E(i)\"\n" << \
                 "plot \"" << filename_out_ds << "\" using 1:2 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();   

    ss.str("");
    ss << "temp/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I << "/gi.plot";
    plot_f.open(ss.str().c_str());

    plot_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"graph/DensityStat_L=" << L << "_PP=" << PP_I << "/gi.jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"i\"\n" << \
                 "set ylabel \"g(i)\"\n" << \
                 "plot \"" << filename_out_ds << "\" using 1:3 title \"landau-wang-omp-" << L << "\" with lines lt rgb \"red\"";

    plot_f.close();  

    // ss.str("");
    // ss << "temp/DensityStat_L=" << L << "/Hi.plot";
    // plot_f.open(ss.str().c_str());

    // plot_f << "#!/usr/bin/gnuplot -persist\n" << \
    //              "set terminal jpeg font arial 12 size 800,600\n" << \
    //              "set output \"graph/DensityStat_L=" << L << "/Hi.jpg\"\n" << \
    //              "set grid x y\n" << \
    //              "set yrange [0:10000000]\n" << \
    //              "set xlabel \"i\"\n" << \
    //              "set ylabel \"H(i)\"\n" << \
    //              "plot \"" << filename_out_ds << "\" using 1:4 title \"landau-wang-" << L << "\" with lines lt rgb \"red\"";

    // plot_f.close();

    ss.str("");
    ss << "graph/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "graph/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "plot_graph_L=" << "3D_" << L << "_PP=" << PP_I << ".sh";

    script_f.open(ss.str().c_str());

    ss.str("");
    ss << "#!/bin/bash\n";
    ss << "gnuplot temp/TermodinamicalStat_L=" << "3D_" << L << "_PP=" << PP_I << "/*.plot\n";
    ss << "gnuplot temp/DensityStat_L=" << "3D_" << L << "_PP=" << PP_I << "/*.plot\n";

    script_f << ss.str();

    // ss.str("");
    // ss << "sh plot_graph_L=" << L << ".sh";

    // chdir(".");
    // system(ss.str().c_str());

    time_e = omp_get_wtime();

    std::cout << "Time: " << time_e - time_b << "'s" << std::endl;

    ss.str("");
    ss << "Time-3D" << L << "-PP-" << PP_I << "_MAXMCS=" << MAX_MCS_COUNT;

    time_f.open(ss.str().c_str());

    time_f << time_e - time_b << "'s" << std::endl;

    time_f.close();

    out_f_td.close();
    out_f_ds.close();

    delete [] filename_out_td;
    delete [] filename_out_ds;

    delete [] E_min;
    delete [] E_max;
    delete [] b_last;

}

int MakeScriptsForAnimation(std::string str)   {

    // Функция создания скриптов для анимации для плотности энергетических состояний

    // Функция создания скриптов для создания анимации для гистограммы

    std::stringstream ss;
    std::ofstream graph_sh;

    if(str == "G")  {
        
        for(int intervals = 0; intervals < PP_I; intervals++)  {
    
            ss.str("");
            ss << "plot_test_g_graph-L=" << "3D_" << L << "_PP=" << PP_I << "-" << intervals << ".sh";
                
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << L << "_PP=" << PP_I << "-" << intervals << \
                            "/graphs/{1..20}.jpg animate-DoS-L=" << L << "_PP=" << PP_I <<"-" << intervals << ".gif\n";
            graph_sh.close();
    
        }
    
        ss.str("");
        ss << "plot_test_g_graph-L=" << "3D_" << L << "_PP=" << PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << L << "_PP=" << PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/DoS-L=" << L << "_PP=" << PP_I << "-AV" << \
                        "/graphs/{1..20}.jpg animate-DoS-L=" << L << "_PP=" << PP_I <<"-AV" << ".gif\n";
        graph_sh.close();
    }

    if(str == "H")  {

        for(int intervals = 0; intervals < PP_I; intervals++)  {

            ss.str("");
            ss << "plot_test_hist_graph-L=" << "3D_" << L << "_PP=" << PP_I << "-" << intervals << ".sh";
        
            graph_sh.open(ss.str().c_str());
            graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << intervals << "/temp/*.plot\n";
            graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << L << "_PP=" << PP_I << "-" << intervals << \
                    "/graphs/{1..200}.jpg animate-Hist-L=" << L << "_PP=" << PP_I << "-" << intervals << ".gif\n";
            graph_sh.close();

        }

        ss.str("");
        ss << "plot_test_hist_graph-L=" << "3D_" << L << "_PP=" << PP_I << "-AV" << ".sh";
    
        graph_sh.open(ss.str().c_str());
        graph_sh << "#!/bin/bash\n" <<  "gnuplot test_g/Hist-L=" << L << "_PP=" << PP_I << "-AV" << "/temp/*.plot\n";
        graph_sh <<  "convert -delay 10 -loop 0 test_g/Hist-L=" << L << "_PP=" << PP_I << "-AV" << \
                            "/graphs/{1..200}.jpg animate-Hist-L=" << L << "_PP=" << PP_I <<"-AV" << ".gif\n";
        graph_sh.close();

    }

    return 0;

}


// inline int neighbour_spins(int **spin, int i, int j)    {

//     int result;

//     if(i==0)    result=spin[L-1][j];    
//     else        result=spin[i-1][j];
//     if(i==L-1)  result+=spin[0][j];
//     else        result+=spin[i+1][j];
//     if(j==0)    result+=spin[i][L-1];
//     else        result+=spin[i][j-1];
//     if(j==L-1)  result+=spin[i][0];
//     else        result+=spin[i][j+1];

//     return result;
// }
