//============================================================================
// Name        : tese.cppss
// Author      : Samuel Moreira
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

// to compile
// g++ -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include *.cpp -o saida -L/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic -lconcert -lcplex -lm -lpthread -g -w
// to execute
// ./saida 3 10000


using namespace std;
#include <sys/time.h>
#include <iostream>
#include "io.h"
#include "ilp.h"
#include "aux.h"


// If you need to update server costs, do it here.
Values set_values() {
	float div =50000;
	Values values; // receive
	values.alfa = 0.05/div; // link revenue
	values.beta = 0.25/div; // core revenue
	values.gama = 0.5/div; // memory revenue

	// cost
	values.delta = 0.01/div; // link cost 0.0015
	values.varepsilon = 0.05/div; // core cost 0.015
	values.zeta = 0.1/div; // memory cost

	//values.w = 1; // xi (custo por imagem)

	values.epsilon = 5/div; // active server cost 35//1//3
	return values;
}


int main(int argc, char *argv[]) {

	int **mat_node;
	int ***mat_link;

	vector<SFC> arrive;
	vector<SFC> wating;
	vector<SFC> finished;
	vector<SFC> faill;

	//new
	
	//if you change the network substrate, update the path here
	string SN_patch = "SN_Compuserve.txt";
	int n = 11;
	int m = 14;



	int rec = atoi(argv[1]);// reconfiguration the reconfiguration type is passed as an argument. 1, to only reconfigure the size of the VNF instances, 2 to reconfigure the instance sizes and routings, and 3 to reconfigure the entire model.
	// 1 images
	// 2 link + imagens
	// 3


	//if you want to work with other requests, change here
	stringstream pa;
	pa <<"SFCsCompuserve_ClassMiX/";
	int num_sfcs = 308;
	
	/*pa <<"SFCsCompuserve_Class3/";
	int num_sfcs = 100;
	
	pa <<"SFCsCompuserve_Class2/";
	int num_sfcs = 125;

	pa <<"SFCsCompuserve_Class1/";
	int num_sfcs = 83;*/

	
	int num_sfcs_feasible = 0;



	string SFCs_patch = pa.str().c_str();

	stringstream espalhamento_arcos;
	stringstream perc_delay;
	stringstream delay_Saida;
	stringstream servidores_ativos;
	stringstream espalhamento_nos;
	stringstream SAC;
	stringstream time_t;

	Values values = set_values();
	// alocation of memory of capacity
	alocation_memory_values(values);





	double gain_final = 0;

	SN sn = io_SN(SN_patch, n);
	sn.m = m;
	arrive = io_SFC(SFCs_patch, num_sfcs, values);
	num_sfcs_feasible = arrive.size();


	srand(time(0));
	double total_time = 0;


	int num_server=0;


	int pc = 0;
	int num_servidores_ativos =0;




	int acceptanceSFCs=0;
	int entrantes=0;



	/*-----------------------------------------------------------------------------------------*/

	int c150In = 0;
	int c300In = 0;
	int c3000In = 0;

	int c150Acc = 0;
	int c300Acc = 0;
	int c3000Acc = 0;

	double profit=0;
	double lucro_corrente=0;


	while ((arrive.size() > 0) || (wating.size() > 0)) {
		stringstream saida_classificador;
		pc++;

		if(pc>atoi(argv[2]))// Time window
			  break;


		// remove in the waiting vector to be finished
		for (int i = 0; i < wating.size(); ++i)
			if ((wating[i].departure_time <= pc) && (wating[i].maped == true)) {
				printf("\nRemove %s t exit:%3d time: %3d\n",wating[i].patch.c_str(), wating[i].departure_time, pc);
				remove(i, wating, finished);
				i = 0;
				ver_viavel(values, wating, sn, lucro_corrente);
			}



		bool need_map = false;
		// Insert in the waiting vector to be processed
		for (int i = 0; i < arrive.size(); ++i)
			if (arrive[i].arrival_time <= pc) {
				printf("patch %s arrival_time:%3d pc: %3d -- max_delay_suported %d\n",
						arrive[i].patch.c_str(), arrive[i].arrival_time, pc,
						arrive[i].max_delay_suported);

				/*SFcs per class arrive count*/
				if (arrive[i].max_delay_suported == 150)
					c150In++;
				else if (arrive[i].max_delay_suported == 300)
					c300In++;
				else
					c3000In++;

				remove(i, arrive, wating);
				i = 0;
				need_map = true;
				pc--;
				entrantes++;
				break;
			}




		vector<float> features_SN_SFC;
		features_SN_SFC.clear();


		if (need_map == true) {
			clock_t tInicio = clock();

			ilp(values, wating, sn, rec,num_server,num_servidores_ativos);

			double tt=(((clock() - tInicio) / (CLOCKS_PER_SEC / 1000)));
			time_t<<tt<<endl;
			total_time += (((clock() - tInicio) / (CLOCKS_PER_SEC / 1000)));

			if (wating[wating.size() - 1].maped == true) {
				perc_delay << ter_percetual_delay(wating[wating.size() - 1], sn)<< ",";

				/*count SFCs from each class in*/
				if(wating[wating.size() - 1].max_delay_suported == 150)
					c150Acc++;
				else if(wating[wating.size() - 1].max_delay_suported==300)
					c300Acc++;
				else
					c3000Acc++;

			}

			if (wating[wating.size() - 1].maped == true) {
				ver_viavel(values, wating, sn, lucro_corrente);
				acceptanceSFCs++;

				for (int ic = 0; ic < wating.size(); ++ic)
					print_SFC(wating[ic], sn);

			}else
				printf("\n Map Faill\n");




		}

		// remove in the waiting vector to faill vector
		for (int i = 0; i < wating.size(); ++i)
			if (wating[i].maped == false) {
				printf("\n..Faill %s\n", wating[i].patch.c_str());
				remove(i, wating, faill);
				i = -1;
			}



		double avg_server_spread=0;
		double avg_server_spreadC=0;
		if (wating.size() > 0){
			avg_server_spread = verifica_usados(wating, sn.n);
			avg_server_spreadC = verifica_usadosC(wating, sn.n);
		}




		espalhamento_arcos<<metrica_espalhamento(wating, sn)<<",";
		delay_Saida<<calcula_delay_medio(wating, sn)<<",";
		espalhamento_nos<<to_string(avg_server_spread)<<",";
		SAC<<avg_server_spreadC<<",";


		for (int p = 0; p < wating.size(); ++p)
			if (wating[p].maped == true)
				profit += wating[p].R;

		gain_final += lucro_corrente;

	}

	perc_delay<<"0";
	espalhamento_arcos<<"0";
	delay_Saida<<"0";
	espalhamento_nos<<"0";
	SAC<<"0";
	stringstream ss;

	ss << rec<< "\t time " << (total_time/1000) << "\t"
			<< "avg accep "<<(float) ((float) acceptanceSFCs / (float) entrantes)
			<< " gain "<< gain_final << "\t"
			<< " profit "<< profit << "\t";
	ss << "\t["<< c150In<<","<<c300In<<","<<c3000In<<"]["<<c150Acc<<","<<c300Acc<<","<<c3000Acc<<"]"
			<< " time window "<<argv[2]<<" reconfiguration mode "<<argv[1];

//uncomment if it is necessary to print the metrics
//plota_exp(time_t.str(), "time_"+to_string(rec));
//	plota_exp(espalhamento_nos.str(), "node_spread__"+to_string(rec));
//	plota_exp(espalhamento_arcos.str(), "arc_spread_"+to_string(rec));
//	plota_exp(delay_Saida.str(), "delay_"+to_string(rec));

//prints a general log of the mappings performed in a file
	plota_exp(ss.str(), "log.txt");


	for(int i =0; i< faill.size();i++)
		cout<<faill[i].patch.c_str()<<endl;


	cout<<"----------\n-------\n---\n-";

	//free_mem(sn, values);

	return 0;
}
