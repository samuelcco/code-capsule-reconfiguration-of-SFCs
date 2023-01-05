#include <string>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <random>
#include <iomanip>
#include <iterator>
#include <fstream>
using namespace std;

struct CPU_MEM {
	int CPU, MEM;
	float custo;
};

bool ver_viavel(Values values, vector<SFC> vector, SN sn, double &foS);
void alocation_memory_values(Values &values);

void alocation_memory_values(Values &values) {
	// alocation of memory of capacity
	values.cap = (CAP**) (malloc(8 * sizeof(CAP*)));
	for (int i = 0; i < 8; i++)
		values.cap[i] = (CAP*) (malloc(9 * sizeof(CAP)));
	for (int i = 0; i < 8; ++i) {
		values.cap[0][i].C = 10000;
		values.cap[1][i].C = 8;
		values.cap[2][i].C = 16;
		values.cap[3][i].C = 32;
		values.cap[4][i].C = 64;
		values.cap[5][i].C = 96;
		values.cap[6][i].C = 10000;
		values.cap[7][i].C = 10000;

		values.cap[0][i].M = 10000;
		values.cap[1][i].M = 475;
		values.cap[2][i].M = 950;
		values.cap[3][i].M = 1200;
		values.cap[4][i].M = 1900;
		values.cap[5][i].M = 3600;
		values.cap[6][i].M = 10000;
		values.cap[7][i].M = 10000;

		values.cap[0][i].custo = 10000;
		values.cap[1][i].custo = 0.361 / 3600; // preço por segundos
		values.cap[2][i].custo = 0.723 / 3600;
		values.cap[3][i].custo = 1.648 / 3600;
		values.cap[4][i].custo = 2.892 / 3600;
		values.cap[5][i].custo = 5.424 / 3600;
		values.cap[6][i].custo = 10000;
		values.cap[7][i].custo = 10000;

	}
	for (int i = 0; i < 7; ++i) {
		values.cap[i][0].M = 10000;
		values.cap[i][0].C = 10000;
	}
}

void fre_mem(int *f1c, int *f2c, int *f3c, int *f4c, int *f5c, int *f6c,
		int *f7c, int *f1m, int *f2m, int *f3m, int *f4m, int *f5m, int *f6m,
		int *f7m) {
	//fo_lucro(wating, values, sn)<<endl;
	free(f1c);
	free(f2c);
	free(f3c);
	free(f4c);
	free(f5c);
	free(f6c);
	free(f7c);
	free(f1m);
	free(f2m);
	free(f3m);
	free(f4m);
	free(f5m);
	free(f6m);
	free(f7m);
}

CPU_MEM imagem_fit(int demanda_cpu, int demanda_mem, Values values) {
	CPU_MEM ret;
	ret.CPU = 0;
	ret.MEM = 0;
	ret.custo = 0;

	if ((demanda_cpu == 0) && (demanda_mem == 0)) // sem demanda
		return ret;

	for (int i = 1; i < 8; ++i)
		if ((demanda_cpu <= values.cap[i][1].C)
				&& (demanda_mem <= values.cap[i][1].M)) {
			ret.MEM = values.cap[i][1].M;
			ret.CPU = values.cap[i][1].C;
			ret.custo = values.cap[i][1].custo;

			/*printf("%d %d\n",demanda_cpu,values.cap[i][1].C);
			 printf("%d %d\n",demanda_mem,values.cap[i][1].M);
			 printf("%f ",values.cap[i][1].custo);
			 getchar();*/
			return ret;
		}

	ret.CPU = 99999999; // valor grande, para rejeitar mapeamento caso de falha de image
	ret.MEM = 99999999;
	ret.MEM = 99999999;
	return ret;
}

bool ver_viavel(Values values, vector<SFC> vec, SN sn, double &foS) {
	foS = 0;
	// nos ativos verificafr banda negativa

	for (int i = 0; i < sn.n; ++i)
		for (int j = 0; j < sn.n; ++j)
			sn.link[i][j].bw_res = sn.link[i][j].bw_ori;

	for (int i = 0; i < vec.size(); ++i)
		for (int j = 0; j < vec[i].virtual_links.size(); ++j) // falha banda
			for (int l = 0; l < vec[i].virtual_links[j].caminho.size(); ++l) {
				sn.link[vec[i].virtual_links[j].caminho[l].u][vec[i].virtual_links[j].caminho[l].v].bw_res -=
						vec[i].virtual_links[j].bw;
				if (sn.link[vec[i].virtual_links[j].caminho[l].u][vec[i].virtual_links[j].caminho[l].v].bw_res
						< 0)
					return false;
			}

	int *f1c = (int*) calloc(sn.n, sizeof(int));
	int *f2c = (int*) calloc(sn.n, sizeof(int));
	int *f3c = (int*) calloc(sn.n, sizeof(int));
	int *f4c = (int*) calloc(sn.n, sizeof(int));
	int *f5c = (int*) calloc(sn.n, sizeof(int));
	int *f6c = (int*) calloc(sn.n, sizeof(int));
	int *f7c = (int*) calloc(sn.n, sizeof(int));

	int *f1m = (int*) calloc(sn.n, sizeof(int));
	int *f2m = (int*) calloc(sn.n, sizeof(int));
	int *f3m = (int*) calloc(sn.n, sizeof(int));
	int *f4m = (int*) calloc(sn.n, sizeof(int));
	int *f5m = (int*) calloc(sn.n, sizeof(int));
	int *f6m = (int*) calloc(sn.n, sizeof(int));
	int *f7m = (int*) calloc(sn.n, sizeof(int));

	for (int i = 0; i < vec.size(); ++i)
		for (int j = 0; j < vec[i].vnfs.size(); ++j)
			if (vec[i].vnfs[j].type_num == 1) {
				f1c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f1m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 2) {
				f2c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f2m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 3) {
				f3c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f3m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 4) {
				f4c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f4m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 5) {
				f5c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f5m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 6) {
				f6c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f6m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			} else if (vec[i].vnfs[j].type_num == 7) {
				f7c[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].core_required;
				f7m[vec[i].vnfs[j].fisico] += vec[i].vnfs[j].mem_required;
			}

	double cost = 0, cc = 0;
	for (int i = 0; i < sn.n; ++i) {
		CPU_MEM t1 = imagem_fit(f1c[i], f1m[i], values);
		CPU_MEM t2 = imagem_fit(f2c[i], f2m[i], values);
		CPU_MEM t3 = imagem_fit(f3c[i], f3m[i], values);
		CPU_MEM t4 = imagem_fit(f4c[i], f4m[i], values);
		CPU_MEM t5 = imagem_fit(f5c[i], f5m[i], values);
		CPU_MEM t6 = imagem_fit(f6c[i], f6m[i], values);
		CPU_MEM t7 = imagem_fit(f7c[i], f7m[i], values);
		cost += t1.custo + t2.custo + t3.custo + t4.custo + t5.custo + t6.custo
				+ t7.custo;

		if ((t1.CPU + t2.CPU + t3.CPU + t4.CPU + t5.CPU + t6.CPU)
				> sn.node[i].vCPU_ori) {
			fre_mem(f1c, f2c, f3c, f4c, f5c, f6c, f7c, f1m, f2m, f3m, f4m, f5m,
					f6m, f7m);
			//cout<<"Não possui CPU"<<endl;
			//getchar();
			return false; // não possui CPU suficiente
		}

		if ((t1.MEM + t2.MEM + t3.MEM + t4.MEM + t5.MEM + t6.MEM)
				> sn.node[i].memory_ori) {
			fre_mem(f1c, f2c, f3c, f4c, f5c, f6c, f7c, f1m, f2m, f3m, f4m, f5m,
					f6m, f7m);

			return false; // não possui CPU suficiente
		}
	}

	int *nos_ativos = (int*) calloc(sn.n, sizeof(int));

	double gain = 0;

	for (int i = 0; i < vec.size(); ++i)
		for (int j = 0; j < vec[i].vnfs.size(); ++j)
			nos_ativos[vec[i].vnfs[j].fisico] = 1; // nós ativos com 1

	for (int j = 0, c = 0; j < sn.n; ++j) // servidores usados
		if (nos_ativos[j] == 1)
			gain -= values.epsilon;
	free(nos_ativos);
	for (int i = 0; i < vec.size(); ++i) {
		gain += vec[i].R; // receita
		gain -= SFC_LC(vec[i], values); // link cost
		gain -= SFC_SC(values, vec[i], sn); //node cost
	}

	gain -= cost;

	fre_mem(f1c, f2c, f3c, f4c, f5c, f6c, f7c, f1m, f2m, f3m, f4m, f5m, f6m,
			f7m);

	foS = gain;
	return true;
}

