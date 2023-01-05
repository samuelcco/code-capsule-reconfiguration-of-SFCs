#include <string>
#include <stdio.h>
#include <vector>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

struct Arco_cluster {
	int u, v;
};

struct Node {
	int index, vCPU_ori, vCPU_res, vCPU_restore, memory_ori, memory_res,
			memory_restore, cluster_bfs;
	double x, y;
	string city, machine;
	double hx;

};

struct Link {
	int dist, o_int, d_int;
	double bw_ori, bw_res, restore, delay;
	bool used;
	string ori_string, des_string;
};

struct SN {
	int n, m;
	double gain, fo;
	vector<Node> node;
	Link **link;
	bool *active_server;
	bool ***w;
	int servers_used, bins_used;
};

struct VNF {
	string type, city;
	double bw_inc_dec, bw, core_required, mem_required, vnf_delay;
	int type_num;
	int index;
	int fisico;
	float lat, lon;
	vector<int> LRC;
};

struct Caminho {
	int u;
	int v;
};

struct CAP {
	int M, C;
	float custo;
};

struct Values {
	float alfa, beta, gama, delta, epsilon, varepsilon, zeta; //,w;
	CAP **cap;
};

struct Arco {
	int o, d;
	float bw;
	vector<Caminho> caminho;
};

struct SFC {
	int num_vnfs, arrival_time, departure_time, duaration, max_delay_suported,
			type, index, cluster;
	vector<int> cluster_bfs;

	bool feasible;
	float espalhamento, initial_bw_required;
	int ori_index, dest_index;
	double R, LC, SC, mapp_delay, hmax, hmin;
	bool maped;
	float lat, lon;
	string ori_city, dest_city, patch;
	vector<VNF> vnfs;
	vector<Arco> virtual_links;

};

//-------------------------------------------------------------------------------------------------------- Headers

void saida_classificador_SFC(const SFC sfc, SN sn, stringstream &saida);
void saida_classificador_SN(SN sn, stringstream &saida);
SN io_SN(string path, int n);
void print_SN(const SN sn);
vector<SFC> io_SFC(string path, int num, Values values);
SFC abrirVN(string patch, Values values, int j);
void print_SFCs(const vector<SFC> vector, SN sn);
void remove(int index, vector<SFC> &vector1, vector<SFC> &vector2);
void plota_exp(string text, string patch);
void calcule_SFC_Revenue(const Values values, SFC &temp);
void print_SFC(const SFC vector, SN sn);
float ter_percetual_delay(const SFC sfc, SN sn);
void calcule_SFC_Fixed_Cost(const Values values, SFC &temp);
float calcula_espalhamento(SFC sfc, int m);
float metrica_espalhamento(const vector<SFC> &wating, const SN &sn);
float calcula_delay_medio(vector<SFC> sfcs, SN sn);
double verifica_usados(vector<SFC> wating, int n);
double verifica_usadosC(vector<SFC> wating, int n);
void aloca_matriz(int k, int **&mat);



//-------------------------------------------------------------------------------------------------------- Functions




double verifica_usadosC(vector<SFC> wating, int n) {
	vector<int> usados;

	for (int i = 0; i < n; ++i)
		usados.push_back(0);

	int num_vnfs_demandado = 0;

	for (int ic = 0; ic < wating.size(); ++ic)
		if (wating[ic].maped == true)
			for (int j = 0; j < wating[ic].vnfs.size(); ++j)
				if (wating[ic].vnfs[j].type_num > 0) {
					usados[wating[ic].vnfs[j].fisico]++;
					num_vnfs_demandado++;
				}

	int num_server_usado = 0;
	for (int i = 0; i < usados.size(); ++i)
		if (usados[i] > 0)
			num_server_usado++;
	return (double) ((double) (num_server_usado * 30)
			/ (double) num_vnfs_demandado);
}

double verifica_usados(vector<SFC> wating, int n) {
	vector<int> usados;

	for (int i = 0; i < n; ++i)
		usados.push_back(0);

	int num_vnfs_demandado = 0;

	for (int ic = 0; ic < wating.size(); ++ic)
		if (wating[ic].maped == true)
			for (int j = 0; j < wating[ic].vnfs.size(); ++j)
				if (wating[ic].vnfs[j].type_num > 0) {
					usados[wating[ic].vnfs[j].fisico]++;
					num_vnfs_demandado++;
				}

	int num_server_usado = 0;
	for (int i = 0; i < usados.size(); ++i)
		if (usados[i] > 0)
			num_server_usado++;
	return (double) ((double) num_server_usado / (double) num_vnfs_demandado);
}

float calcula_delay_medio(vector<SFC> sfcs, SN sn) {
	float delay = 0;
	int it = 0;
	if (sfcs.size() == 0)
		return delay;

	for (int v = 0; v < sfcs.size(); ++v)
		if (sfcs[v].maped == true) {
			it++;
			for (int i = 0; i < sfcs[v].virtual_links.size(); ++i)
				for (int j = 0; j < sfcs[v].virtual_links[i].caminho.size();
						++j)
					delay +=
							sn.link[sfcs[v].virtual_links[i].caminho[j].u][sfcs[v].virtual_links[i].caminho[j].v].delay;
			for (int i = 0; i < sfcs[v].vnfs.size(); ++i)
				delay += sfcs[v].vnfs[i].vnf_delay;

		}

	return ((double) ((double) delay / (double) it));
}

float metrica_espalhamento(const vector<SFC> &wating, const SN &sn) {
	float espalhamento = 0;
	for (int ic = 0; ic < wating.size(); ++ic) {
		espalhamento += calcula_espalhamento(wating[ic], sn.m);
		//cout << ic << "	" << espalhamento << endl;
	}
	if (espalhamento > 0)
		espalhamento = ((float) (espalhamento) / (float) (wating.size()));

	return espalhamento;
}

float calcula_espalhamento(SFC sfc, int m) {
	float ret = 0;
	for (int i = 0; i < sfc.virtual_links.size(); ++i)
		ret += sfc.virtual_links[i].caminho.size();

	return (float) ((float) ret / (float) (2 * m));
}

void aloca_matriz(int k, int **&mat) {
	mat = (int**) (malloc(k * sizeof(int*)));
	for (int i = 0; i < k; i++)
		mat[i] = (int*) ((malloc(200 * sizeof(int))));
	for (int i = 0; i < k; ++i)
		for (int j = 0; j < 200; ++j)
			mat[i][j] = -9;
}

double SFC_LC(SFC sfc, const Values &values) {
	double gain = 0;
	for (int i = 0; i < sfc.virtual_links.size(); ++i)
		for (int j = 0; j < sfc.virtual_links[i].caminho.size(); ++j)
			gain += sfc.virtual_links[i].bw * values.delta;
	return gain;
}

double SFC_SC(const Values &values, SFC sfcs, SN &sn) {
	double gain = 0;

	for (int k = 0; k < sfcs.vnfs.size(); ++k)
		gain += sfcs.vnfs[k].core_required * values.varepsilon;

	for (int k = 0; k < sfcs.vnfs.size(); ++k)
		gain += sfcs.vnfs[k].mem_required * values.zeta;

	return gain;
}

void plota_exp(string text, string patch) {
	FILE *arq;
	arq = fopen(patch.c_str(), "a+");
	fprintf(arq, "%s\n", text.c_str());
	fclose(arq);
}

void remove(int index, vector<SFC> &vector1, vector<SFC> &vector2) {
	// remove the element whit index from vector one to vector two
	//cout<<"Mooving "<<vector1[index].patch<<endl;
	vector2.push_back(vector1[index]);
	vector1.erase(vector1.begin() + index);
}

double calcula_delayy(SFC sfcs, SN sn) {
	double delay = 0;
	for (int i = 0; i < sfcs.virtual_links.size(); ++i)
		for (int j = 0; j < sfcs.virtual_links[i].caminho.size(); ++j)
			delay +=
					sn.link[sfcs.virtual_links[i].caminho[j].u][sfcs.virtual_links[i].caminho[j].v].delay;

	for (int i = 0; i < sfcs.vnfs.size(); ++i)
		delay += sfcs.vnfs[i].vnf_delay;

	//cout<<delay<<endl;
	return delay;
}

float ter_percetual_delay(const SFC sfc, SN sn) {
	return 100
			* (1
					+ ((sfc.mapp_delay - sfc.max_delay_suported)
							/ sfc.max_delay_suported));
}

void print_SFC(const SFC sfc, SN sn) {

	cout << "------------------------------->\n" << "Origem " << sfc.ori_index
			<< " Detino " << sfc.dest_index << "\nPacth:" << sfc.patch
			<< "	Max delay:" << sfc.max_delay_suported << "	Delay:"
			<< sfc.mapp_delay << "	Mapeada: (" << sfc.maped
			<< ") AVG max delay:"
			<< 100
					* (1
							+ ((sfc.mapp_delay - sfc.max_delay_suported)
									/ sfc.max_delay_suported)) << "\nRevenue	"
			<< sfc.R << "\nServer Cost	" << sfc.SC << "\nArrive:	"
			<< sfc.arrival_time << " Duration: " << sfc.duaration
			<< " Departure: " << sfc.departure_time << " Initial Band: "
			<< sfc.initial_bw_required << "	Feasible:	" << sfc.feasible
			<< "	Cluster: ";
	for (int j = 0; j < sfc.cluster_bfs.size(); ++j)
		cout << sfc.cluster_bfs[j] << ",";

	cout << endl;

	if (sfc.maped == false)
		return;

	cout << "Severs:________\n";

	for (int j = 0; j < sfc.vnfs.size(); ++j)
		printf("%d	%15s	\%20s (%d) Fisico: %3d	(%3.1f	%3.1f)\n",
				sfc.vnfs[j].index, sfc.vnfs[j].city.c_str(),
				sfc.vnfs[j].type.c_str(), sfc.vnfs[j].type_num,
				sfc.vnfs[j].fisico, sfc.vnfs[j].lon, sfc.vnfs[j].lat);

	cout << "Links:________\n";
	for (int j = 0; j < sfc.virtual_links.size(); ++j) {
		printf("Source	%3d (%3d)	Destiny	%3d (%3d) bw	%2.2f	Patch:	",
				sfc.virtual_links[j].o, sfc.vnfs[sfc.virtual_links[j].o].fisico,
				sfc.virtual_links[j].d, sfc.vnfs[sfc.virtual_links[j].d].fisico,
				sfc.virtual_links[j].bw);

		for (int l = 0; l < sfc.virtual_links[j].caminho.size(); ++l)
			printf("(%3d,%3d) ", sfc.virtual_links[j].caminho[l].u,
					sfc.virtual_links[j].caminho[l].v);
		printf("\n");
	}

}

void saida_classificador_SFC(const SFC sfc, SN sn, stringstream &saida) {
	saida.str("");

	saida << sfc.ori_index << "	" << sfc.dest_index << "	"
			<< sfc.max_delay_suported << "	" << sfc.R << "	" << sfc.duaration
			<< "	" << sfc.initial_bw_required << "	" << sfc.vnfs.size() << "	";

	// servidores
	int regulariza_saida = 0;
	for (int j = 0; j < sfc.vnfs.size(); ++j, ++regulariza_saida)
		saida << sfc.vnfs[j].type.c_str() << "	";

	for (; regulariza_saida < 9; ++regulariza_saida) {
		saida << 1 << "	";
	}
}

void saida_classificador_SN(SN sn, stringstream &saida) {

	for (int j = 0; j < sn.node.size(); ++j) {
		saida << sn.node[j].memory_res << "	";
		saida << sn.node[j].vCPU_res << "	";
	}

	// links bw residual
	for (int i = 0; i < sn.n; i++)
		for (int j = 0; j < sn.n; j++)
			if (sn.link[i][j].used == 1)
				saida << sn.link[i][j].bw_res << "	";

}

void print_SFCs(const vector<SFC> vector, string msg, SN sn) {
	cout << "\n\n" << msg << endl;
	for (int i = 0; i < vector.size(); ++i) {

		cout
				<< "------------------------------------------------------------------------->	"
				<< "Delay request:" << calcula_delayy(vector[i], sn)
				<< "	Max delay:" << vector[i].max_delay_suported << "\n" << msg
				<< "\n" << vector[i].patch << "	" << "Mapeada:	"
				<< vector[i].maped << " Revenue	" << vector[i].R
				<< " 	Fesible:	" << vector[i].feasible;

		cout << endl;

		if (vector[i].maped) {

			cout << vector[i].vnfs.size() << "	"
					<< vector[i].virtual_links.size() << "	"
					<< vector[i].arrival_time << "	" << vector[i].duaration
					<< "	" << vector[i].max_delay_suported << endl;

			cout << "Severs:________\n";
			for (int j = 0; j < vector[i].vnfs.size(); ++j)
				cout << vector[i].vnfs[j].bw << "	"
						<< vector[i].vnfs[j].bw_inc_dec << "	"
						<< vector[i].vnfs[j].core_required << "	"
						<< vector[i].vnfs[j].mem_required << "	"
						<< vector[i].vnfs[j].vnf_delay << "	"
						<< vector[i].vnfs[j].index << "	Físico: "
						<< sn.node[vector[i].vnfs[j].fisico].city << "	("
						<< vector[i].vnfs[j].fisico << ")	"
						<< vector[i].vnfs[j].type << "	"
						<< vector[i].vnfs[j].city << endl;

			cout << "Arcos:________\n";
			for (int j = 0; j < vector[i].virtual_links.size(); ++j) {
				cout << vector[i].virtual_links[j].o << "	"
						<< vector[i].virtual_links[j].d << "	"
						<< vector[i].virtual_links[j].bw << "		";

				for (int l = 0; l < vector[i].virtual_links[j].caminho.size();
						++l) {
					cout
					//<< sn.node[vector[i].virtual_links[j].caminho[l].u].city
					<< "(" << vector[i].virtual_links[j].caminho[l].u << "-"
							//<< sn.node[vector[i].virtual_links[j].caminho[l].v].city
							<< "" << vector[i].virtual_links[j].caminho[l].v
							<< "), ";
				}
				cout << endl;
			}
		}
	}
}

void calcule_SFC_Revenue(const Values values, SFC &temp) {
	temp.R = 0;

	for (int i = 0; i < temp.vnfs.size(); ++i) {
		temp.R += temp.vnfs[i].core_required * values.beta;
		temp.R += temp.vnfs[i].mem_required * values.gama;
	}
	for (int kl = 0; kl < temp.virtual_links.size(); ++kl)
		temp.R += temp.virtual_links[kl].bw * values.alfa;
}

void calcule_SFC_Fixed_Cost(const Values values, SFC &temp) {
	temp.SC = 0;

	for (int i = 0; i < temp.vnfs.size(); ++i) {
		temp.SC += temp.vnfs[i].core_required * values.varepsilon;
		temp.SC += temp.vnfs[i].mem_required * values.zeta;
	}

}

SFC abrirVN(string patch, Values values, int j) {
	SFC temp;
	float flow = 0;

	string line;
	ifstream myfile(patch.c_str());

	if (myfile.is_open()) {
		getline(myfile, line);
		istringstream iss(line);
		iss >> temp.num_vnfs;
		iss >> temp.arrival_time;
		iss >> temp.duaration;
		temp.index = j;
		temp.departure_time = temp.duaration + temp.arrival_time;
		iss >> temp.max_delay_suported;

		//temp.max_delay_suported=temp.max_delay_suported*10;
		iss >> temp.initial_bw_required;
		//temp.initial_bw_required=25;
		temp.patch = patch;
		temp.maped = false;
		temp.mapp_delay = 0;
		flow = temp.initial_bw_required;

		// get origin
		getline(myfile, line);
		istringstream isss(line);
		isss >> temp.ori_index;
		isss >> temp.type;
		isss >> temp.ori_city;
		isss >> temp.lon;
		isss >> temp.lat;
		//cout<<lat<<"\n"<<lon<<endl;

		VNF origem;
		origem.bw = temp.initial_bw_required;
		origem.bw_inc_dec = 1;
		origem.core_required = 0;
		origem.mem_required = 0;
		origem.type = "source";
		origem.vnf_delay = 0;
		origem.index = temp.ori_index;
		origem.city = temp.ori_city;
		origem.type_num = temp.type;
		origem.lat = temp.lat;
		origem.fisico = -1;
		origem.lon = temp.lon;

		/*cout<<origem.index<<endl;

		 for (int i = 0; i < origem.binary.size(); ++i)
		 cout<<origem.binary[i]<<" ";

		 getchar();*/

		temp.vnfs.push_back(origem);

		//get VNFs
		for (int i = 0; i < temp.num_vnfs; ++i) {
			getline(myfile, line);
			istringstream iss(line);
			VNF vnf_temp;
			iss >> vnf_temp.core_required;
			iss >> vnf_temp.mem_required;
			iss >> vnf_temp.bw_inc_dec;
			iss >> vnf_temp.vnf_delay;
			iss >> vnf_temp.type_num;
			iss >> vnf_temp.type;
			vnf_temp.lat = 0;
			vnf_temp.lon = 0;
			vnf_temp.city = "VNF";
			vnf_temp.fisico = -1;
			vnf_temp.index = -1;
			temp.vnfs.push_back(vnf_temp);
		}

		// get destiny
		getline(myfile, line);
		istringstream issss(line);
		issss >> temp.dest_index;
		issss >> temp.type;
		issss >> temp.dest_city;
		issss >> temp.lon;
		issss >> temp.lat;

		VNF destiny;
		destiny.bw = 0;
		destiny.bw_inc_dec = 1;
		destiny.fisico = -1;
		destiny.core_required = 0;
		destiny.mem_required = 0;
		destiny.type = "destiny";
		destiny.vnf_delay = 0;
		destiny.index = temp.dest_index;
		destiny.city = temp.dest_city;
		destiny.type_num = temp.type;
		destiny.lat = temp.lat;
		destiny.lon = temp.lon;
		temp.vnfs.push_back(destiny);

	}

	for (int i = 0; i < temp.vnfs.size() - 1; ++i) {
		temp.vnfs[i].bw = flow;
		flow = flow * temp.vnfs[i + 1].bw_inc_dec;
	}

	//links
	for (int i = 0; i < temp.vnfs.size() - 1; ++i) {
		Arco arc_temp;
		arc_temp.o = i;
		arc_temp.d = i + 1;
		arc_temp.bw = temp.vnfs[i].bw;
		temp.virtual_links.push_back(arc_temp);
	}

	calcule_SFC_Revenue(values, temp);
	calcule_SFC_Fixed_Cost(values, temp);

	/*cout<<"Receita: "<<temp.R<<endl;
	 cout<<"Despesa fixa: "<<temp.SC<<endl;



	 getchar();*/

	return temp;
}

vector<SFC> io_SFC(string path, int num, Values values) {
	vector<SFC> arrive;

	int j;
	for (j = 0; j < num; j++) {
		stringstream ss;
		ss << path << "req" << j << ".txt";
		arrive.push_back(abrirVN(ss.str(), values, j));
	}

	return arrive;
}

SN io_SN(string path, int n) {
	SN sn;
	sn.n = n;
	// alocation of memory
	sn.link = (Link**) malloc(sn.n * sizeof(Link*));
	for (int i = 0; i < sn.n; i++) {
		sn.link[i] = (Link*) malloc(sn.n * sizeof(Link));
		for (int j = 0; j < sn.n; j++) {
			sn.link[i][j].used = false;
			sn.link[i][j].delay = 0;
		}
	}

	sn.active_server = (bool*) malloc(sn.n * sizeof(bool));
	for (int i = 0; i < sn.n; ++i)
		sn.active_server[i] = false;

	sn.w = (bool***) malloc(sn.n * sizeof(bool**));
	for (int i = 0; i < sn.n; i++) {
		sn.w[i] = (bool**) malloc(8 * sizeof(bool*));
		for (int j = 0; j < 8; j++) {
			sn.w[i][j] = (bool*) malloc(8 * sizeof(bool));
			for (int k = 0; k < 8; k++)
				sn.w[i][j][k] = false;
		}
	}

	string line;
	ifstream myfile(path);
	if (myfile.is_open()) {

		getline(myfile, line);
		stringstream ss(line);
		ss >> sn.n;
		int m;
		ss >> m;
		ss.str("");

		//cout << sn.n << "	"<< endl;

		// physical nodes open
		for (int i = 0; i < sn.n; ++i) {
			getline(myfile, line);
			//cout<<line<<endl;

			stringstream sss(line);
			Node temp;
			sss >> temp.index;
			sss >> temp.x;
			sss >> temp.y;
			sss >> temp.vCPU_ori;
			temp.vCPU_res = temp.vCPU_ori;
			sss >> temp.memory_ori;
			temp.memory_res = temp.memory_ori;
			sss >> temp.machine;
			sss >> temp.city;

			//cout << temp.index << "	" << temp.x << "	" << temp.y << "	"<< temp.city << "	" << temp.vCPU_ori << "	" << temp.memory_ori<< "	" << temp.machine << "	" << endl;
			sn.node.push_back(temp);
			sss.str("");

		}

		// physical links open
		for (int i = 0; i < m; ++i) {

			getline(myfile, line);
			stringstream ssss(line);
//			cout << line << endl;

			Link temp;
			ssss >> temp.o_int;

			ssss >> temp.o_int;
			ssss >> temp.d_int;
			ssss >> temp.bw_ori;
			temp.bw_res = temp.bw_ori;
			ssss >> temp.dist;
			ssss >> temp.delay;
			temp.delay = temp.delay * 10;
			//ss >> temp.ori_string;
			//ss >> temp.des_string;

			sn.link[temp.o_int][temp.d_int].used = true;
			sn.link[temp.d_int][temp.o_int].used = true;

			sn.link[temp.o_int][temp.d_int].bw_ori = temp.bw_ori;
			sn.link[temp.d_int][temp.o_int].bw_ori = temp.bw_ori;

			sn.link[temp.o_int][temp.d_int].bw_res = temp.bw_ori;
			sn.link[temp.d_int][temp.o_int].bw_res = temp.bw_ori;

			sn.link[temp.o_int][temp.d_int].dist = temp.dist;
			sn.link[temp.d_int][temp.o_int].dist = temp.dist;

			sn.link[temp.o_int][temp.d_int].delay = temp.delay;
			sn.link[temp.d_int][temp.o_int].delay = temp.delay;

			sn.link[temp.o_int][temp.d_int].o_int = temp.o_int;
			sn.link[temp.d_int][temp.o_int].o_int = temp.o_int;

			sn.link[temp.o_int][temp.d_int].d_int = temp.d_int;
			sn.link[temp.d_int][temp.o_int].d_int = temp.d_int;
			ssss.str("");

		}
	}

	return sn;
}

void print_SN(const SN sn) {
	for (int i = 0; i < sn.node.size(); ++i)
		cout << i << ")	" << sn.node[i].index << "	" << sn.node[i].x << "	"
				<< sn.node[i].y << "	" << sn.node[i].memory_ori << "	"
				<< sn.node[i].vCPU_ori << "	" << sn.node[i].machine << "	"
				<< sn.node[i].city << endl;
	int cell = 0;
	for (int i = 0; i < sn.n; ++i) {
		cout << endl;
		for (int j = 0; j < sn.n; ++j)
			if (sn.link[i][j].used == false)
				cout << "-";
			else {
				cout << sn.link[i][j].used;
				cell++;
			}
	}
	cout << "\nCelulas usadas	" << cell << endl;
	cout << sn.link[195][196].delay << "	" << sn.link[195][196].o_int << "	"
			<< sn.link[195][196].d_int << "	"
			<< sn.node[sn.link[195][196].o_int].city << "	"
			<< sn.node[sn.link[195][196].d_int].city << endl;
}
