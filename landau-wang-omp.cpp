/*
 * landau-wang.cpp
 *
 *  Created on: 24 мая 2014 г.
 *      Author: nlare
 * 
 *  Данная версия ещё довольно далека от состояния готовности, пересылаю в качестве некого промежуточного этапа работы.
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
#define energy(b) (2*(b)-2.0*(L*L))
#define PP_I 2
#define L 16

// int neighbour_spins(int,int);

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
    int it_count;
    double overlap;
    int h;
    int *E_min, *E_max;
    int *b_last;
    // int count;
    double buffer; // Переменная для сохранения g(i) текущей реплики
    double exchange; // Переменная для сохранения g(i) реплики, с которой происходит обмен

    bool trigger = true;
    bool convergence_trigger[PP_I];

    int hist_average[top_b];
    int div_averaging[top_b];

    CRandomMersenne rg(13617235);

    srand(time(NULL));

    std::stringstream ss;
    std::ofstream test_g_f, graph_g_f, graph_sh_g;

    ss.str("");
    ss << "test_g";
    boost::filesystem::create_directories(ss.str().c_str());

    f = 2.7182818284;  // В работе Ландау-Ванга было указано значение "f" равное экспоненте 
    f_min = 1.000001;  // Данная переменная должна быть около единицы
    min_steps = 10000;
    skip = 10000;
    flat_threshold = 0.8;
    overlap = 0.75;
    it_count = 0;
    h = 1;

    struct _spins   {
    
        int spin[L][L];

    }   system_of[PP_I];

    top_b=L*L;

    struct _massive   {
        
        double g[4*L*L];
        int hist[4*L*L];

    }   massive[PP_I];

    double g_average[4*L*L];
    double g_normalized[4*L*L];

    // for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        for(int i = 0; i <= 2*L*L; i++)    {
            g_average[i] = 0.0;
        }
    // }

    for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
     
        for(int i = 0; i < L; i++)    {
            for(int j = 0; j < L; j++)    {
                system_of[pp_i].spin[i][j] = 1;
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

        for(int i = 0; i <= 2*L*L; i++)    {
            massive[pp_i].g[i] = 1.0;
        }

        for(int i = 0; i <= 2*L*L; i++)    {
            massive[pp_i].hist[i] = 1.0;
        }

    }

    if(L%2!=0) top_b=2*(L*(L-1))+1;
    else top_b=2*(L*L);

    E_min = new int [PP_I];
    E_max = new int [PP_I];

    b_last = new int [PP_I];

    double value = (top_b - top_b*overlap)/PP_I;

    for(int rank = 1; rank <= PP_I; rank++)   {

        E_min[rank-1] = value*(rank-1);
        E_max[rank-1] = top_b*overlap + value*rank;

        std::cout << std::fixed << std::setprecision(4) << "E_min[" << rank-1 << "]=" << E_min[rank-1] \
                  << ", E_max[" << rank-1 << "]=" << E_max[rank-1] << ", overlap=" << overlap \
                  << std::endl;
    }    

    // b = 0;
    for(int i = 0; i < PP_I; i++)   {
      b_last[i] = E_min[i];  
      std::cout << "b_last[" << i << "] = " <<  b_last[i] << std::endl;
    } 


    // E_max[1] = 127;

    // #ifdef DEBUG
    // std::cout << std::setprecision(10) \
    //           << "Energy level range: top_b = " << top_b << std::endl \
    //           << "Precision: f_min = " << f_min << std::endl; 
    // #endif

    omp_set_num_threads(PP_I);

    time_b = omp_get_wtime();

    std::cout << "Begin of estimation:" << std::endl;

    // Считаем пока "f" не примет минимальное значение
    while(f > f_min)    {

        #pragma omp parallel for \
        firstprivate(f,min_steps,skip,flat_threshold,h,E_min,E_max,top_b) \
        private(b_new,b,ln_f)  \
        shared(system_of,massive,b_last,buffer,convergence_trigger,g_average,div_averaging,hist_average)
        for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        
        int seed = 1 + rand() % 10000;
        CRandomMersenne Mersenne(seed);
        Mersenne.RandomInit(seed);

        int mcs;
        int count, n;
        int c = skip+1;
        double prob;    // Вероятность изменения энергетического уровня

        // int E_diff = E_max[pp_i] - E_min[pp_i];

        double g[top_b];

        ln_f = log(f);  // Для вычислений будем использовать логарифм множителя "f" 
        count = 1;  // Если гистограмма не стала плоской, останется равной единице и цикл продолжится.
        mcs = 0;
        n = 0;
        // b = 0;
        // #pragma omp flush
        // {
        b = b_last[pp_i];

        if(b == 0)    {
            b = E_min[pp_i];
        }

        int sum_spins = 0;

        for(int i = 0; i < L; i++)    {
            for(int j = 0; j < L; j++)    {
                sum_spins += system_of[pp_i].spin[i][j];
            }
        }

        #pragma omp flush(massive)

        // if(omp_get_thread_num()==0)
        #pragma omp critical
        // std::cout << "b = " << b << "\t, sum_spins = " << sum_spins << "\t, rank = " << pp_i << std::endl;

        // }
        convergence_trigger[pp_i] = false;
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

        for (int i = 0; i < top_b; i++)   {
            massive[pp_i].hist[i] = 0;
        }

        do {

        if(mcs < 20)   {
            goto m3;
        }

        for(int i = 0; i < L; i++)   {
            for(int j = 0; j < L; j++)  {
                m1:
                int ci = Mersenne.IRandom(0, L-1);
                int cj = Mersenne.IRandom(0, L-1);

                if(system_of[pp_i].spin[ci][cj] == 0) goto m1;  // Исключаем немагнитные спины

                int neighbour_spins = 0;
                #pragma omp flush(system_of)
                if(ci==0)   neighbour_spins=system_of[pp_i].spin[L-1][cj];
                else        neighbour_spins=system_of[pp_i].spin[ci-1][cj];
                if(ci==L-1) neighbour_spins+=system_of[pp_i].spin[0][cj];
                else        neighbour_spins+=system_of[pp_i].spin[ci+1][cj];
                if(cj==0)   neighbour_spins+=system_of[pp_i].spin[ci][L-1];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj-1];
                if(cj==L-1) neighbour_spins+=system_of[pp_i].spin[ci][0];
                else        neighbour_spins+=system_of[pp_i].spin[ci][cj+1];

                // #pragma omp flush(system_of)
                b_new = b + system_of[pp_i].spin[ci][cj]*neighbour_spins;   // Считаем новое состояние

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
                if((b_new < E_max[pp_i]) && (b_new > E_min[pp_i]))   {   // Если энергия в допустимых пределах
                    
                    prob = exp(massive[pp_i].g[b]-massive[pp_i].g[b_new]);  // Вероятность принятия нового энергетического состояния
                    // prob = massive[pp_i].g[b]/massive[pp_i].g[b_new];
                    // #pragma omp critical
                    // if(omp_get_thread_num() == 1)
                    // std::cout << prob << "\n";
                    if(prob >= 1.0 || (double)(Mersenne.IRandom(1,10000)/10000.0) < prob)    {

                        b = b_new;
                        // #pragma omp flush(system_of)
                        // #pragma omp critical
                        system_of[pp_i].spin[ci][cj] *= -1;

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
                    n++;
                }

                // }
            }
        }

        m3:

        mcs++;
        c++;

        // #pragma omp critical
        // {
        //     std::cout << "mcs[" << pp_i << "]:" << mcs << std::endl;
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
            int overlap_interval_begin, overlap_interval_end;
            int random_index_x, random_index_y;

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

            // int first_not_null_g_for_pp_i, first_not_null_g_for_exchange_replica; 
            // double delta_G = 0.0;
            #define REPLICAEXHANGE 
            #ifdef REPLICAEXHANGE
            #pragma omp critical
            {           
                // for(int i = overlap_interval_begin; i < overlap_interval_end; i++)
                {

                // for(int i = overlap_interval_begin; i < overlap_interval_end; i+=2)  {

                    random_index_x = Mersenne.IRandom(0,top_b);
                    // random_index_y = Mersenne.IRandom(0,top_b);
                    random_index_y = random_index_x;

                    while(random_index_x%2!=0)  {
                        random_index_x = Mersenne.IRandom(0,top_b);
                        if(random_index_x%2==0) break;
                    }

                    while(random_index_y%2!=0)  {
                        random_index_y = Mersenne.IRandom(0,top_b);
                        if(random_index_y%2==0) break;
                    }

                    // random_index_y = random_index_x - diff_between_g;

                    // if(random_index_x%2!=0)
                    // std::cout << "random_index_x = " << random_index_x << std::endl;
                    // std::cout << "random_index_y = " << random_index_y << std::endl;

                    #pragma omp flush(massive)
                    // if(convergence_trigger[num_of_exchange_replica]==false)
                    // if(massive[pp_i].hist[random_index_x] != 0 && massive[num_of_exchange_replica].hist[random_index_y] != 0)
                    if((random_index_x > overlap_interval_begin) && (random_index_x < overlap_interval_end)\
                        && (random_index_y > overlap_interval_begin) && (random_index_y < overlap_interval_end))    {

                        // if(massive[pp_i].hist[random_index_x]!=0 && massive[pp_i].hist[random_index_x]!=0 \
                            // && massive[num_of_exchange_replica].hist[random_index_y]!=0 && massive[num_of_exchange_replica].hist[random_index_y]!=0)
                        #pragma omp flush(random_index_x,random_index_y)
                        // {
                            prob = (massive[pp_i].g[random_index_x]*massive[num_of_exchange_replica].g[random_index_y])/ \
                                   (massive[pp_i].g[random_index_y]*massive[num_of_exchange_replica].g[random_index_x]);

                        // std::cout << "random_index_x = " << random_index_x << std::endl;
                        // std::cout << "random_index_y = " << random_index_y << std::endl;                        
                        
                            // std::cout << std::fixed << std::setprecision(5)\
                            //             << std::endl << "---------------------------------------------------------------------------------------------------------------------------------------" << std::endl\
                            //             << "massive[" << pp_i << "].g[" << random_index_x << "]=" << massive[pp_i].g[random_index_x] \
                            //             << ", massive[" << num_of_exchange_replica << "].g[" << random_index_y << "]=" << massive[num_of_exchange_replica].g[random_index_y]\
                            //             << std::endl\
                            //             << "massive[" << pp_i << "].g[" << random_index_y << "]=" << massive[pp_i].g[random_index_y]\
                            //             << ", massive[" << num_of_exchange_replica << "].g[" << random_index_x << "]=" << massive[num_of_exchange_replica].g[random_index_x]\
                            //             << "\nprob=" << prob << std::endl\
                            //             << "---------------------------------------------------------------------------------------------------------------------------------------" << std::endl;

                        // }

                        // prob = 1.0 - prob;

                        // std::cout << "prob = " << prob << std::endl;

                        // prob = 0.0;

                        if(prob >= 1.0 || (double)(Mersenne.IRandom(1,10000)/10000.0) < prob) { //&& (fabs(massive[pp_i].g[random_index_y] - massive[num_of_exchange_replica].g[random_index_y])) < 100.0) {
                            // std::cout << "diff = " << (massive[pp_i].g[random_index_y] - massive[num_of_exchange_replica].g[random_index_y]) << std::endl;
                            // #pragma omp critical
                            // std::cout << "Gen = " << (double)(Mersenne.IRandom(1,10000)/10000.0) << std::endl;
                            // #pragma omp flush
                            // {
                            // for(int i = overlap_interval_begin; i <= overlap_interval_end; i++) {
                            //     if(massive[pp_i].hist[i]!=0)    {
                            //         first_not_null_g_for_pp_i = i;
                            //         break;
                            //     }
                            // }
                            // }

                            // #pragma omp flush
                            // {
                            // for(int i = overlap_interval_begin; i <= overlap_interval_end; i++) {
                            //     if(massive[num_of_exchange_replica].hist[i]!=0)    {
                            //         first_not_null_g_for_exchange_replica = i;
                            //         break;
                            //     }
                            // }
                            // }

                            // delta_G = massive[pp_i].g[first_not_null_g_for_exchange_replica] - massive[num_of_exchange_replica].g[first_not_null_g_for_exchange_replica];
                            // std::cout << "delta_G = " << delta_G << std::endl;



                            // std::cout << "TESTING VAR's:" << std::endl;
                            // for(int i = 0; i < PP_I; i++)
                            // std::cout << "massive #" << pp_i << std::endl;
                                // for(int j = 0; j < overlap_interval_end;j++)   {
                                    // std::cout << pp_i << ":g[" << j << "]=" << massive[pp_i].g[j] << "\t\t"\
                                              // << num_of_exchange_replica << ":g[" << j << "]=" << massive[num_of_exchange_replica].g[j]\
                                              // << std::endl;
                                // }
                            
                            // #pragma omp critical
                            // {
                            
                            // std::cout << std::fixed << "ln_f=" << ln_f << ", count = " << count << ", prob = " << prob\
                                      // << "\tg[" << random_index_x << "]=" << massive[pp_i].g[random_index_x]\
                                      // << " <--> g[" << random_index_x << "]=" << massive[num_of_exchange_replica].g[random_index_x]\
                                      // << ", Thread #" << omp_get_thread_num() << ", dG = " << diff_between_g << std::endl;
                        


                            // ---------------- EXHANGE BETWEEN ALL G[i] INSIDE INTERVAL -----------------------
                            // std::cout << "init exchange between THREAD #" << pp_i << " and #" << num_of_exchange_replica << " in overlapping G[i] interval. " << std::endl;
                            // for(int i = overlap_interval_begin; i < overlap_interval_end; i++)    {
                            //     // #pragma omp flush(massive, buffer)
                            //     if(massive[pp_i].hist[i]!=0)    {
                            //         // std::cout << "massive[" << pp_i << "].g[" << i << "]=" << massive[pp_i].g[i]\
                            //                   // << " <--> g[" << num_of_exchange_replica << "].g[" << (i - diff_between_g)\
                            //                   // << "]=" << massive[num_of_exchange_replica].g[(i - diff_between_g)] << std::endl;
                            //         buffer = massive[pp_i].g[i];
                            //         massive[pp_i].g[i] = massive[num_of_exchange_replica].g[i];
                            //         massive[num_of_exchange_replica].g[i] = buffer;

                            //         // if(massive[pp_i].g[random_index_y] \
                            //         // < massive[num_of_exchange_replica].g[random_index_y])
                            //         // massive[pp_i].g[i] += ln_f;
                            //         // massive[pp_i].hist[i] += 1;
                            //     }
                            // }
                            // ---------------- EXHANGE BETWEEN ALL G[i] INSIDE INTERVAL -----------------------


                            // ---------------- EXHANGE BETWEEN SOME RANDOM INDEXES OF G[i] -----------------------

                            // if(abs(massive[pp_i].g[random_index_x] - massive[num_of_exchange_replica].g[random_index_y]) < 100.0)    {

                                std::cout << "init exchange between THREAD #" << pp_i << " and #" << num_of_exchange_replica << " : " << random_index_x << "[increment to " << massive[pp_i].g[random_index_x] << "]" << " <-> " << random_index_y << "[" << massive[num_of_exchange_replica].g[random_index_y] << "]" << std::endl;
                                // #pragma omp flush(massive,buffer,random_index_x,random_index_y,ln_f)
                                // buffer = massive[pp_i].g[random_index_x];
                                // massive[pp_i].g[random_index_x] = massive[num_of_exchange_replica].g[random_index_y];
                                // massive[num_of_exchange_replica].g[random_index_y] = buffer;
                                // if(massive[pp_i].g[random_index_x] < massive[num_of_exchange_replica].g[random_index_y])   {
                                    // massive[pp_i].g[random_index_x] += ln_f;
                                    // massive[pp_i].hist[random_index_x] += 1;
                                // }
                                if(massive[pp_i].g[random_index_x] < massive[num_of_exchange_replica].g[random_index_y]) {
                                    massive[pp_i].g[random_index_x] += ln_f;
                                    massive[pp_i].hist[random_index_x] += 1;
                                } 
                                else {
                                    massive[num_of_exchange_replica].g[random_index_y] += ln_f;
                                    massive[num_of_exchange_replica].hist[random_index_y] += 1;
                                }

                            // }

                            // ---------------- EXHANGE BETWEEN SOME RANDOM INDEXES OF G[i] -----------------------


                            // }

                            // }

                            // }
                            // std::cout << "------------------------------------------------------" << std::endl;
                            // std::cout << "After Exhcange:" << std::endl;
                            // std::cout << "------------------------------------------------------" << std::endl;

                            // for(int i = 0; i < PP_I; i++)
                            // std::cout << "massive #" << pp_i << std::endl;
                            // for(int j = 0; j < overlap_interval_end;j++)   {
                                // std::cout << pp_i << ":g[" << j << "]=" << massive[pp_i].g[j] << "\t\t"\
                                          // << num_of_exchange_replica << ":g[" << j << "]=" << massive[num_of_exchange_replica].g[j]\
                                          // << std::endl;
                            // }
        
                        }   else   {
                            // std::cout << "hist#" << i << " or " << i+2 << " is null" << std::endl;
                            // prob = 0.0;
                            // count++;
                        }

                        // if((prob != 1.0) && (prob != 0))    {
                            
                            // std::cout << std::fixed << std::setprecision(5) << "count = " << count << ", prob = " << prob << std::endl;\
                            //           << ", 1:G[" << i << "]=" << massive[pp_i].g[i] \
                            //           << ", 2:G[" << i+2 << "]=" << massive[num_of_exchange_replica].g[i+2] \
                            //           << ", 1:G[" << i+2 << "]=" << massive[pp_i].g[i+2] \
                            //           << ", 2:G[" << i << "]=" << massive[num_of_exchange_replica].g[i] \
                                      // << std::endl;
                        // }
                        
                    }                    
                    // if(omp_get_thread_num() == 0)   {
                    //     massive[1].g[i] = 0.0;
                        // std::cout << "massive[num_of_exchange_replica]=" << massive[1].g[i] << std::endl;
                    // }
                // }
                }
            }
            #endif

            int h_count = 0;
            double h_delt = 0.0;
            double h_sum = 0.0;

            // #pragma omp critical
            
            for(int i = E_min[pp_i]; i < E_max[pp_i]; i++) {

                if(massive[pp_i].hist[i] != 0)    {
                    h_sum += (double)massive[pp_i].hist[i]/min_steps;
                    h_count++;
                }

            }

            // #pragma omp flush
            count = 0;  // Выходим из цикла по окончанию

            for (int i = E_min[pp_i]; i < E_max[pp_i]; i++)    {
                if(massive[pp_i].hist[i] != 0)    {
                    h_delt = (double)(massive[pp_i].hist[i]/(h_sum/h_count))/min_steps;
                    if(h_delt <= flat_threshold || h_delt >= 1.0 + flat_threshold) {count = 1;}

                    // if(h_delt < flat_threshold) {
                    //     std::cout << std::endl << "---------------------------" << std::endl;
                    //     std::cout << i << ":" << h_delt << std::endl;
                    //     std::cout << "---------------------------" << std::endl;
                    // }

                    // else goto m2;
                //     if((massive[pp_i].g[i] < E_max[pp_i]) && (massive[pp_i].g[i] > E_max[pp_i]-4 ) ) {
                //     std::cout << i << ":" << h_delt << std::endl;
                // }
                }
            }

            // #pragma omp flush
            // if(count == 1 && h_delt > flat_threshold) goto m2;
            // printf("h_delt = %f\n",h_delt);
            // std::cout << std::setprecision(8) << "ln_f=" << ln_f << ", flat_threshold = " << flat_threshold << ", h_delt = " << h_delt << ", count = " << count << std::endl;
        
        }   else count = 1;

        }   while(count);

        m2:

        b_last[pp_i] = b;
        // std::cout << "b_last[" << pp_i << "]= " << b_last[pp_i] << std::endl;
        convergence_trigger[pp_i] = true;

        #pragma omp critical
        std::cout << std::fixed << std::setprecision(8) << "L - " << L << ": f = " << f << ", ln_f = " << ln_f << ", mcs = " << mcs << ", thread #" << omp_get_thread_num() << std::endl;

        }

        #pragma omp barrier

        // std::cout << "Beyond a barrier" << std::endl;

        // #pragma omp flush(massive)

        // double g_averaging[top_b];
        // int div_averaging[top_b];

        // for(int i = 0; i < top_b; i++)  {
        //     g_averaging[i] = 0.0;
        //     div_averaging[i] = 0.0;
        // }

        // for(int i = 0; i < top_b; i++)  {

        //     div_averaging[i] = PP_I;        

        // }

        // for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        //     int j = 0;
        //     for(int i = 0; i < top_b; i++)  {

        //     if(massive[pp_i].hist[i]!=0)   
        //         g_averaging[j] += massive[pp_i].g[i];

        //     if(massive[pp_i].g[i] == 1.0) {
        //         div_averaging[i] -= 1;
        //         // std::cout << "g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
        //     }

        //     j++;

        //     }
        // }
        // #pragma omp flush(E_min,E_max)
        // for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        //     for(int i = E_min[pp_i]; i < E_max[pp_i]; i++) {

        //         if(massive[pp_i].hist[i]!=0)    {
        //             massive[pp_i].g[i] = g_averaging[i]/div_averaging[i];
        //         }

        //     }

        // }

        // std::cout << "OUT massive[pp_i].g[0]:" << std::endl;
        // for(int i = E_min[0]; i < E_max[0]; i++)    {
        //     std::cout << "massive[0].g[" << i << "]=" << massive[0].g[i] << std::endl;
        // }

        // std::cout << "OUT massive[pp_i].g[1]:" << std::endl;
        // for(int i = E_min[1]; i < E_max[1]; i++)    {
        //     std::cout << "massive[1].g[" << i << "]=" << massive[0].g[i] << std::endl;
        // }

        if(omp_get_thread_num() == 0)   {

        double max_in_pp_i;
        int in_count = 0; 

        // std::cout << "In 0 thread section" << std::endl;
        // std::cout << "top_b = " << top_b << std::endl;

        for(int i = 0; i < top_b; i++)  {
            g_average[i] = 0.0;
            hist_average[i] = 0.0;
            div_averaging[i] = PP_I;
            // std::cout << in_count++ << std::endl;
        }

        // std::cout << "Before flush" << std::endl;

        #pragma omp flush(g_average,massive,div_averaging)

        for(int i = 0; i < top_b; i++)  {
            // max_in_pp_i = 0;
            for(int rank = 0; rank < PP_I; rank++)  {
                if(massive[rank].hist[i]!=0)    {
                // -------------- CHOOSE MAX G[E] IN ALL REPLICAS -----------------
                // if(massive[pp_i].g[i] > max_in_pp_i)   {    
                //     max_in_pp_i = massive[pp_i].g[i];
                //     g_average[i] = max_in_pp_i;
                // }
                // -------------- CHOOSE MAX G[E] IN ALL REPLICAS -----------------
                // std::cout << "!" << std::endl;
                // -------------- AVERAGING ALL REPLICAS G[E] -----------------
                g_average[i] += massive[rank].g[i];
                // g_average[i] = ;
                hist_average[i] += massive[rank].hist[i];
                // -------------- AVERAGING ALL REPLICAS G[E] -----------------
                }
                if(massive[rank].g[i] == 1.0) {
                    div_averaging[i] -= 1;
                }
            }
        }

        // #pragma omp flush(g_average,massive,div_averaging)

        // std::cout << "Averaging #0 complete" << std::endl;

        for(int i = 0; i < top_b; i++)  {
            if(!(i%2)) g_average[i]/=div_averaging[i];
            // if(!(i%2)) std::cout << "G[" << i << "]=" << g_average[i] << std::endl;
            // if(!(i%2)) std::cout << "Mas[" << i << "]=" << massive[0].g[i] << std::endl;
        }

        // std::cout << "Averaging #1 complete" << std::endl;

        // for(int i = 0; i < top_b; i++)     {
        //     for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        //         if(hist_average[pp_i][i]!=0)  {
        //             std::cout << "i:" << i << ":" << hist_average[pp_i][i] << ": " << div_averaging[i] << std::endl;
        //             hist_average[pp_i][i]/=div_averaging[i];
        //         }
        //     }
        // }

        std::cout << "Mas = AV" << std::endl;

        for(int i = 0; i < top_b; i++)  {
            for(int pp_i = 0; pp_i < PP_I; pp_i++)
                massive[pp_i].g[i] = g_average[i];
        }

        int first_not_null_g;
        double min_g_value;
        
        it_count++;

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I;
        boost::filesystem::create_directories(ss.str().c_str());

        ss << "/" << it_count << ".dat";
        // std::cout << "write to " << ss.str() << std::endl;

        test_g_f.open(ss.str().c_str());

        for(int i = 4; i < top_b; i++)  {
            if((i!=2*top_b-1))    {
                test_g_f << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << g_average[i] << "\n";
                // std::cout << "massive.g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
            }
        }

        test_g_f.close();

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I << "/temp";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "L=" << L << "_PP=" << PP_I << "/temp/" << it_count << ".plot";
        graph_g_f.open(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-L=" << L << "_PP=" << PP_I << "/graphs";
        boost::filesystem::create_directories(ss.str().c_str());

        ss.str("");
        ss << "test_g/DoS-" << "L=" << L << "_PP=" << PP_I << "/" << it_count << ".dat";

        graph_g_f << "#!/usr/bin/gnuplot -persist\n" << \
                 "set terminal jpeg font arial 12 size 800,600\n" << \
                 "set output \"test_g/DoS-L=" << L << "_PP=" << PP_I << "/graphs/" << it_count << ".jpg\"\n" << \
                 "set grid x y\n" << \
                 "set xlabel \"i\"\n" << \
                 "set ylabel \"G(i)\"\n" << \
                 "plot \"" << ss.str() << "\" using 1:3 title \"landau-wang-" << L << "-iteration-" << it_count << "\" with lines lt rgb \"red\"";

        graph_g_f.close();

        }


        f = pow(f, 0.5);    // Изменяем множитель

    }   

    std::cout << "Ending estimation of G." << std::endl;
    std::cout << "Begin Averaging of G:" << std::endl;

    // std::cout << "something" << std::endl;


    // for(int pp_i = 0; pp_i < PP_I; pp_i++)  {
        // for(int i = 0; i <= top_b; i++)  {

            // if(massive[pp_i].hist[i]!=0)
                // g_average[i] += massive[pp_i].g[i];

            // if(massive[1].hist[18]==0)
                // std::cout << "!" << std::endl;

            // if(massive[pp_i].g[i] == 1.0) {
                // div_averaging[i] -= 1;
                // std::cout << "g[" << i << "]=" << massive[pp_i].g[i] << std::endl;
            // }

        // }
    // }

    // std::cout << "OUT massive[pp_i].g[0]:" << std::endl;
    // for(int i = E_min[0]; i < E_max[0]; i++)    {
    //     std::cout << "massive[0].g[" << i << "]=" << massive[0].g[i] << std::endl;
    // }

    // std::cout << "OUT massive[pp_i].g[1]:" << std::endl;
    // for(int i = E_min[1]; i < E_max[1]; i++)    {
    //     std::cout << "massive[1].g[" << i << "]=" << massive[0].g[i] << std::endl;
    // }

    double min_in_ge = g_average[L*L]; // index may be anything else (not L*L)

    for(int i = 0; i < top_b; i++)  {
        if(hist_average[i]!=0)  {
            if(g_average[i] < min_in_ge)
            min_in_ge = g_average[i];
        }
    }

    // -------------- FOR READISTRIBUTED G[E] -----------------
    // for(int i = 0; i < top_b; i++)  {
    //     if(hist_average[i]!=0)
    //     g_normalized[i] = g_average[i] - min_in_ge;
    // }
    // -------------- FOR READISTRIBUTED G[E] -----------------

    // -------------- FOR AVERAGED G[E] -----------------    
    // for(int i = 0; i < top_b; i++)  {
    //     if(hist_average[i]!=0)
    //     g_average[i] = g_average[i] - min_in_ge;
    // }
    // -------------- FOR AVERAGED G[E] ----------------- 

    for(int i = 0; i < top_b; i++)  {

        // g_average[i]/=div_averaging[i];
        // if(g_average[i] > 1.0)
        if(L == 8)  {
            if(massive[0].hist[i]!=0 || massive[1].hist[i]!=0)
            std::cout << std::fixed << "g[" << i << "]=" << g_average[i] << ":" << div_averaging[i]\
                  << "\t" << massive[0].g[i] << "\t" << massive[1].g[i]\
                  << "\t" << massive[0].g[i] - massive[0].g[4] << "\t"\
                  << massive[1].g[i] - massive[1].g[20] << "\t" << g_average[i] - min_in_ge << std::endl;
        }

        if(L == 16 && PP_I == 2)  {
            if(massive[0].hist[i]!=0 || massive[1].hist[i]!=0)
            std::cout << std::fixed << "g[" << i << "]=" << g_average[i] << ":" << div_averaging[i]\
                  << "\t" << massive[0].g[i] << "\t" << massive[1].g[i]\
                  << "\t" << massive[0].g[i] - massive[0].g[4] << "\t"\
                  << massive[1].g[i] - massive[1].g[68] << "\t" << g_average[i] - min_in_ge << std::endl;
        }

        if(L == 16 && PP_I == 4)  {
            if(massive[0].hist[i]!=0 || massive[1].hist[i]!=0 || massive[2].hist[i]!=0 || massive[3].hist[i]!=0)
            std::cout << std::fixed << "g[" << i << "]=" << g_average[i] << ":" << div_averaging[i]\
                  << "\t" << massive[0].g[i] << "\t" << massive[1].g[i]\
                  << "\t" << massive[2].g[i] << "\t" << massive[3].g[i]\
                  << "\t" << massive[0].g[i] - massive[0].g[4] << "\t"\
                  << massive[1].g[i] - massive[1].g[68] << "\t" << g_average[i] - min_in_ge << std::endl;
        }

        if(L > 16 && PP_I > 2)  {
            if(massive[0].hist[i]!=0 || massive[1].hist[i]!=0)
            std::cout << std::fixed << "g[" << i << "]=" << g_average[i] << ":" << div_averaging[i]\
                  << "\t" << massive[0].g[i] << "\t" << massive[1].g[i]\
                  << "\t" << massive[0].g[i] - massive[0].g[4] << "\t"\
                  << massive[1].g[i] - massive[1].g[68] << "\t" << g_average[i] - min_in_ge << std::endl;
        }

    }

    g_average[2*L*L] = min_in_ge;   


    std::cout << "min_in_g[E] = " << min_in_ge << std::endl;

    std::cout << "End Averaging of G." << std::endl;

    ss.str("");
    ss << "plot_test_g_graph-L=" << L << "_PP=" << PP_I << ".sh";

    graph_sh_g.open(ss.str().c_str());
    graph_sh_g << "#!/bin/bash\n" <<  "gnuplot test_g/DoS-L=" << L << "_PP=" << PP_I << "/temp/*.plot\n";
    graph_sh_g <<  "convert -delay 100 -loop 0 test_g/DoS-L=" << L << "_PP=" << PP_I <<  \
                    "/graphs/{1..20}.jpg animate-DoS-L=" << L << "_PP=" << PP_I << ".gif\n";
    graph_sh_g.close();

    double EE, EE2, GE, Ut, Ft, St, Ct, lambdatemp, lambda;

    std::ofstream out_f_td, out_f_ds, plot_f, script_f, time_f;

    char * filename_out_td = new char [100];
    char * filename_out_ds = new char [100];

    ss.str("");
    ss << "results/TermodinamicalStat_L=" << L << "_PP=" << PP_I << ".dat";

    strcpy(filename_out_td ,ss.str().c_str());

    out_f_td.open(filename_out_td);
    if(!out_f_td) std::cout << "Cannot open " << filename_out_td << ".Check permissions or free space";
    out_f_td << "T\tUt\tFt\tSt\tCt\n";

    ss.str("");
    ss << "results/DensityStat_L=" << L << "_PP=" << PP_I << ".dat";

    strcpy(filename_out_ds, ss.str().c_str());

    out_f_ds.open(filename_out_ds, std::ios::out);
    if(!out_f_ds) std::cout << "Cannot open " << filename_out_ds << ".Check permissions or free space";
    out_f_ds << "i\tE(i)\tg[i]\n";

    std::cout << "Begin estimation for T..." << std::endl; 
    // TEST
    // for(int i = 0; i < top_b; i++)  {

    //     std::cout << i << ":" << g_average[i] << std::endl;

    // }

    for(double T = 0.01; T <= 8; T += 0.01)  {

        EE = 0;
        EE2 = 0;
        GE = 0;

        lambda = 0;
        lambdatemp = 0;

        for(int i = 0; i < top_b; i++)  {
            if((i!=0) && i!=L*L-1 && (hist_average[i]!=0) && g_average[i]==g_average[i])    {
                lambdatemp = g_average[i] - energy(i)/T;
                if(lambdatemp > lambda) lambda = lambdatemp;
            }
        }

        for(int i = 0; i < top_b; i++) {
            if((i!=1) && (i!=L*L-1) && (hist_average[i]!=0) && g_average[i]==g_average[i])    {
                EE += energy(i)*exp(g_average[i]-(energy(i))/T-lambda);
                EE2 += energy(i)*energy(i)*exp(g_average[i]-(energy(i))/T-lambda);
                GE += exp(g_average[i]-energy(i)/T-lambda);
            }
        }

        Ut = EE/GE;
        Ft = -T*lambda-(T)*log(GE);
        St = (Ut-Ft)/T;
        Ct = ((EE2/GE)-Ut*Ut)/(T*T);

        // if((Ut==Ut)==true&&(Ft==Ft)==true&&(St==St)==true&&(Ct==Ct)==true) // Не пишем NaN 
        out_f_td << std::fixed << std::setprecision(4) << T << "\t" << Ut/(L*L) << "\t" << Ft/(L*L) << "\t" << St/(L*L) << "\t" << Ct/(L*L) << "\n";

    }

    for(int i = 0; i <= top_b; i++)  {
        if((i!=2*(L*L)-1) && i%2==0)    {
            if((g_average[i]-min_in_ge) >= 0 && (g_average[i]-min_in_ge)==(g_average[i]-min_in_ge))
            out_f_ds << std::fixed << std::setprecision(6) << i << "\t" << energy(i) << "\t" << g_average[i] - min_in_ge << "\t" << hist_average[i] << "\n";
        }
    }

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ut.plot";
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
    ss << "temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ft.plot";
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
    ss << "temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/St.plot";
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
    ss << "temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/Ct.plot";
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
    ss << "temp/DensityStat_L=" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "temp/DensityStat_L=" << L << "_PP=" << PP_I << "/Ei.plot";
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
    ss << "temp/DensityStat_L=" << L << "_PP=" << PP_I << "/gi.plot";
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
    ss << "graph/TermodinamicalStat_L=" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "graph/DensityStat_L=" << L << "_PP=" << PP_I;

    boost::filesystem::create_directories(ss.str().c_str());

    ss.str("");
    ss << "plot_graph_L=" << L << "_PP=" << PP_I << ".sh";

    script_f.open(ss.str().c_str());

    ss.str("");
    ss << "#!/bin/bash\n";
    ss << "gnuplot temp/TermodinamicalStat_L=" << L << "_PP=" << PP_I << "/*.plot\n";
    ss << "gnuplot temp/DensityStat_L=" << L << "_PP=" << PP_I << "/*.plot\n";

    script_f << ss.str();

    // ss.str("");
    // ss << "sh plot_graph_L=" << L << ".sh";

    // chdir(".");
    // system(ss.str().c_str());

    time_e = omp_get_wtime();

    std::cout << "Time: " << time_e - time_b << "'s" << std::endl;

    ss.str("");
    ss << "Time-" << L << "-PP-" << PP_I;

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