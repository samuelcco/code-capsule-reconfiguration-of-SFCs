#include <string>
#include <stdio.h>
#include <iostream>
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
#include <iomanip>
#include <iostream>
#include <fstream>
#include <ilcplex/ilocplex.h>
#define MAXCARACTERES 20

const long int BIGM = 10000;
const long double EPS = 0.000001;

using namespace std;

ILOSTLBEGIN
typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<NumVarMatrix> NumVar3Matrix;
typedef IloArray<NumVar3Matrix> NumVar4Matrix;
typedef IloArray<NumVar4Matrix> NumVar5Matrix;
bool ilp(Values values, vector<SFC> &vector, SN &sn, int lock, int &num_server, int &num_servidores_ativos);
bool in(int item, vector<int> vector);
void free_mem(const SN &sn, const Values &values);
void FO_lucro(int V, IloExpr &fo, IloNumVarArray &yVar, NumVar4Matrix &xVar,
		Values &values, IloNumVarArray &sVar, int B, int F, NumVar3Matrix &wVar,
		NumVar3Matrix &zVar, vector<SFC> &vector, SN &sn);

void free_mem(const SN &sn, const Values &values) {
	//free mem
	for (int i = 0; i < sn.n; i++)
		free(sn.link[i]);
	free(sn.link);
	free(sn.active_server);
	for (int i = 0; i < sn.n; i++)
		for (int j = 0; j < 8; j++)
			free(sn.w[i][j]);
	for (int i = 0; i < sn.n; i++)
		free(sn.w[i]);
	free(sn.w);
	for (int i = 0; i < 8; i++)
		free(values.cap[i]); // delete array within matrix
	free(values.cap);
}

void FO_lucro(int V, IloExpr &fo, IloNumVarArray &yVar, NumVar4Matrix &xVar,
		Values &values, IloNumVarArray &sVar, int B, int F, NumVar3Matrix &wVar,
		NumVar3Matrix &zVar, vector<SFC> &vector, SN &sn) {

	for (int v = 0; v < V; ++v)	// SFC link revenue R
		fo += yVar[v] * vector[v].R;

	//LC
	for (int v = 0; v < V; ++v)
		// SFC link cost LC
		for (int kl = 0; kl < vector[v].virtual_links.size(); kl++)
			for (int i = 0; i < sn.n; i++)
				for (int j = 0; j < sn.n; j++)
					if (sn.link[i][j].used == 1)
						fo -= vector[v].virtual_links[kl].bw * xVar[v][kl][i][j]
								* values.delta; //* yVar[v]

				// cost for acitve bin server active SC
	for (int i = 0; i < sn.n; ++i)
		for (int b = 0; b < B; b++)
			for (int f = 0; f < F; f++)
				fo -= wVar[i][b][f] * values.cap[b][2].custo;

	//SC
	for (int i = 0; i < sn.n; ++i)
		// cost for maintaining server active SC
		fo -= sVar[i] * values.epsilon;

	for (int v = 0; v < V; ++v)
		// cost for core server active SC
		for (int k = 0; k < vector[v].vnfs.size(); k++)
			for (int i = 0; i < sn.n; ++i)
				fo -= zVar[v][k][i] * vector[v].vnfs[k].core_required
						* values.varepsilon;

	//fo -= yVar[v] * vector[v].vnfs[k].core_required* values.varepsilon;
	for (int v = 0; v < V; ++v)
		// cost for memory server active SC
		for (int k = 0; k < vector[v].vnfs.size(); k++)
			for (int i = 0; i < sn.n; ++i)
				fo -= zVar[v][k][i] * vector[v].vnfs[k].mem_required
						* values.zeta;
}

bool ilp(Values values, vector<SFC> &vector, SN &sn, int lock,int &num_server, int &num_servidores_ativos) {
	int B = 8;
	int F = 8;
	clock_t tInicio = clock();
	tInicio = clock();

	//free mem

	int V = vector.size();
	try {

		IloEnv env;
		IloModel modelo(env);
		string varName;
		IloCplex cplex(env);

		// variables X, for link mapping
		NumVar4Matrix xVar(env, V);
		for (int v = 0; v < V; ++v) {			//v
			xVar[v] = NumVar3Matrix(env, vector[v].virtual_links.size());
			for (int kl = 0; kl < vector[v].virtual_links.size(); kl++) {//k,l
				xVar[v][kl] = NumVarMatrix(env, sn.n);
				for (int i = 0; i < sn.n; i++) {			//i
					xVar[v][kl][i] = IloNumVarArray(env, sn.n);
					for (int j = 0; j < sn.n; j++) {			//j
						xVar[v][kl][i][j] = IloBoolVar(env);
						if (sn.link[i][j].used == 1) {
							char varname[MAXCARACTERES];
							sprintf(varname, "x_%d_%d_%d_%d", v, kl, i, j);
							xVar[v][kl][i][j].setName(varname);
						}
					}
				}
			}
		}

		// variables Z, for servers
		NumVar3Matrix zVar(env, V);
		for (int v = 0; v < V; ++v) {			//v
			zVar[v] = NumVarMatrix(env, vector[v].vnfs.size());
			for (int k = 0; k < vector[v].vnfs.size(); k++) {			//k
				zVar[v][k] = IloNumVarArray(env, sn.n);
				for (int i = 0; i < sn.n; i++) {			//i
					zVar[v][k][i] = IloBoolVar(env);
					char varname[MAXCARACTERES];
					sprintf(varname, "z_%d_%d_%d", v, k, i);
					zVar[v][k][i].setName(varname);
				}
			}
		}

		// variable Y, for SFC accept
		IloNumVarArray yVar(env, V);
		for (int v = 0; v < V; ++v) {
			yVar[v] = IloBoolVar(env);
			char varname[MAXCARACTERES];
			sprintf(varname, "y_%d", v);
			yVar[v].setName(varname);
		}

		// variable S, for used node
		IloNumVarArray sVar(env, sn.n);
		for (int i = 0; i < sn.n; ++i) {
			sVar[i] = IloBoolVar(env);
			char varname[MAXCARACTERES];
			sprintf(varname, "s_%d", i);
			sVar[i].setName(varname);
		}

		// variables W, for bin used
		NumVar3Matrix wVar(env, sn.n);
		for (int i = 0; i < sn.n; ++i) {
			wVar[i] = NumVarMatrix(env, B);
			for (int b = 0; b < B; b++) {
				wVar[i][b] = IloNumVarArray(env, F);
				for (int f = 0; f < F; f++) {
					wVar[i][b][f] = IloBoolVar(env);
					char varname[MAXCARACTERES];
					sprintf(varname, "w_%d_%d_%d", i, b, f);
					wVar[i][b][f].setName(varname);
				}
			}
		}

		//-----------------------------------------------------------------------------------------------------------------------------------------------------

		IloExpr fo(env);
		FO_lucro(V, fo, yVar, xVar, values, sVar, B, F, wVar, zVar, vector, sn);
		modelo.add(IloMaximize(env, fo));
		fo.end();

		// trava ativas
		for (int v = 0; v < V; ++v)
			if (vector[v].maped == true) {
				if (lock == 1) {
					modelo.add(yVar[v] == 1);
					for (int j = 0; j < vector[v].vnfs.size(); ++j)
						modelo.add(zVar[v][j][vector[v].vnfs[j].fisico] == 1);

					for (int kl = 0; kl < vector[v].virtual_links.size(); kl++)
						for (int l = 0;l < vector[v].virtual_links[kl].caminho.size();++l)
							modelo.add(xVar[v][kl][vector[v].virtual_links[kl].caminho[l].u][vector[v].virtual_links[kl].caminho[l].v]== 1);
				}

				if (lock == 2) {
					modelo.add(yVar[v] == 1);
					for (int j = 0; j < vector[v].vnfs.size(); ++j)
						modelo.add(zVar[v][j][vector[v].vnfs[j].fisico] == 1);

				}

				if (lock == 3)
					modelo.add(yVar[v] == 1);



			}



		// Constraints 1
		for (int i = 0; i < sn.n; ++i) {
			IloExpr r1(env);
			for (int b = 0; b < B; b++)
				for (int f = 0; f < F; f++)
					r1 += wVar[i][b][f] * values.cap[b][f].C;
			modelo.add(r1 <= sn.node[i].vCPU_ori);
			r1.end();
		}

		// Constraints 2
		for (int i = 0; i < sn.n; ++i) {
			IloExpr r2(env);
			for (int b = 0; b < B; b++)
				for (int f = 0; f < F; f++)
					r2 += wVar[i][b][f] * values.cap[b][f].M;
			modelo.add(r2 <= sn.node[i].memory_ori);
			r2.end();
		}




		// Constraints 3
		for (int f = 0; f < F; ++f)
			for (int i = 0; i < sn.n; ++i) {

				IloExpr r3l(env);
				for (int v = 0; v < V; v++)
					for (int k = 0; k < vector[v].vnfs.size(); k++)
						if (vector[v].vnfs[k].type_num != 0)
							if (vector[v].vnfs[k].type_num == f)
								r3l += zVar[v][k][i]
										* vector[v].vnfs[k].mem_required;

				IloExpr r3d(env);
				for (int b = 0; b < B; b++)
					r3d += wVar[i][b][f] * values.cap[b][f].M;

				modelo.add(r3l <= r3d);
				r3d.end();
				r3l.end();
			}

		// Constraints 4
		for (int f = 0; f < F; ++f)
			for (int i = 0; i < sn.n; ++i) {

				IloExpr r4l(env);
				for (int v = 0; v < V; v++)
					for (int k = 0; k < vector[v].vnfs.size(); k++)
						if (vector[v].vnfs[k].type_num != 0)
							if (vector[v].vnfs[k].type_num == f)
								r4l += zVar[v][k][i]
										* vector[v].vnfs[k].core_required;
				IloExpr r4d(env);
				for (int b = 0; b < B; b++)
					r4d += wVar[i][b][f] * values.cap[b][f].C;

				modelo.add(r4l <= r4d);

				r4d.end();
				r4l.end();
			}

		// Constraints 5
		for (int v = 0; v < V; ++v)
			for (int k = 0; k < vector[v].vnfs.size(); k++)
				if (vector[v].vnfs[k].type_num != 0) {			//VNF
					IloExpr r5(env);
					for (int i = 0; i < sn.n; i++)
						r5 += zVar[v][k][i];
					modelo.add(r5 == yVar[v]);
					r5.end();
				}

		// Constraints 6
		for (int v = 0; v < V; ++v)
			for (int k = 0; k < vector[v].vnfs.size(); k++)
				if (vector[v].vnfs[k].type_num == 0) {			//terminal
					IloExpr r6(env);
					for (int i = 0; i < sn.n; i++)
						if (i == vector[v].vnfs[k].index)				//VNF
							r6 += zVar[v][k][i];

					modelo.add(r6 == yVar[v]);
					r6.end();
				}

		// Constraints 7
		for (int v = 0; v < V; ++v)
			for (int i = 0; i < sn.n; i++) {
				IloExpr r7(env);
				for (int k = 0; k < vector[v].vnfs.size(); k++)
					r7 += zVar[v][k][i];
				modelo.add(r7 <= 1);
				r7.end();
			}

		// Constraints 8
		for (int v = 0; v < V; ++v)
			for (int i = 0; i < sn.n; i++) {
				IloExpr r8(env);
				for (int k = 0; k < vector[v].vnfs.size(); k++)
					r8 += zVar[v][k][i];
				modelo.add(r8 <= sVar[i]);
				r8.end();
			}

		// Constraints 9
		for (int i = 0; i < sn.n; i++)
			for (int j = 0; j < sn.n; j++)
				if (sn.link[i][j].bw_ori > 1)
					if (sn.link[i][j].used == true) {
						IloExpr r9(env);
						for (int v = 0; v < V; ++v)
							for (int kl = 0;
									kl < vector[v].virtual_links.size(); kl++)

								r9 += xVar[v][kl][i][j]
										* vector[v].virtual_links[kl].bw;

						modelo.add(r9 <= sn.link[i][j].bw_ori);
						r9.end();
					}

		// Constraints 10
		for (int v = 0; v < V; ++v) {

			IloExpr r10(env);
			for (int kl = 0; kl < vector[v].virtual_links.size(); kl++)
				for (int i = 0; i < sn.n; i++)
					for (int j = 0; j < sn.n; j++)
						if (sn.link[i][j].bw_ori > 1)
							if (sn.link[i][j].used == true)
								r10 += sn.link[i][j].delay * xVar[v][kl][i][j];

			for (int i = 0; i < sn.n; i++)
				for (int k = 0; k < vector[v].vnfs.size(); k++)
					if (vector[v].vnfs[k].type_num != 0)
						r10 += zVar[v][k][i] * vector[v].vnfs[k].vnf_delay;

			modelo.add(r10 <= (vector[v].max_delay_suported));
			r10.end();
		}

		// Constraints 11
		for (int v = 0; v < V; ++v)
			for (int kl = 0; kl < vector[v].virtual_links.size(); kl++)
				for (int i = 0; i < sn.n; i++) {

					IloExpr r11l(env);
					for (int j = 0; j < sn.n; j++)
						if (sn.link[i][j].used == 1)
							if (sn.link[i][j].bw_res > 0.1)
								r11l += xVar[v][kl][i][j] * 1;

					for (int h = 0; h < sn.n; h++)
						if (sn.link[h][i].used == 1)
							if (sn.link[h][i].bw_res > 0.1)
								r11l += xVar[v][kl][h][i] * -1;

					IloExpr r11r(env);
					r11r += zVar[v][vector[v].virtual_links[kl].o][i] * 1;

					r11r += zVar[v][vector[v].virtual_links[kl].d][i] * (-1);

					modelo.add(r11l == r11r);
					r11l.end();
					r11r.end();
				}



		for (int i = 0; i < sn.n; ++i)
			for (int f = 0; f < F; f++) {
				IloExpr r12(env);
				for (int b = 0; b < B; b++)
					r12 += wVar[i][b][f];
				modelo.add(r12 <= 1);
				r12.end();

			}

		cplex.extract(modelo);
		//cplex.exportModel("modelo.lp");

		try {
			cplex.setParam(IloCplex::Param::Threads, 1); // para uma thread
			//cplex.setParam(IloCplex::Param::Emphasis::Memory,1);
			//cplex.setParam(IloCplex::Param::WorkMem,2048);
			//cplex.setParam(IloCplex::Param::DetTimeLimit, 10000); // para uma thread
			//cplex.setParam(IloCplex::SimDisplay, 1);
			//cplex.setParam(IloCplex::BarDisplay, 1); //No progress information
			//cplex.setParam(IloCplex::NetDisplay, 1); //Network logging display indicator
			//cplex.setOut(env.getNullStream());
			//cplex.setParam(IloCplex::Param::TimeLimit, 9000.0);			//600
			//cplex.setParam(IloCplex::EpInt,0.0);// arrendonda ineiros 0/1

			cplex.solve();

		} catch (int e) {
			cout << "ERROR SOOLVE";
			getchar();
			return false;
		}

		if ((cplex.getStatus() == IloAlgorithm::Unknown)
				|| (cplex.getStatus() == IloAlgorithm::Infeasible)) {//Feasible,Infeasible<unKnow
			xVar.end();
			wVar.end();
			yVar.end();
			zVar.end();

			cplex.end();
			env.end();
			cout << "Cplex not optmal and SFC not mapped";
			//getchar();
			return false;
		}

		cout << "Linhas (restrições):	" << cplex.getNrows() << endl;
		cout << "Colunas (variaveis):	" << cplex.getNcols() << endl;
		cout << "Ótimo " << cplex.getStatus() << endl;
		cout << "Size " << vector.size() << endl;
		sn.fo = cplex.getObjValue();
		cout << "FO:" << sn.fo << "..\n\n";

		for (int v = 0; v < V; ++v) {
			if (cplex.getValue(yVar[v]) > 0.99) {
				cout << "\n";
				//print_SFC(vector[v], sn);
				cout << vector[v].patch << "	" << cplex.getValue(yVar[v])
						<< endl;
				vector[v].maped = true;

				for (int k = 0; k < vector[v].vnfs.size(); k++)
					for (int i = 0; i < sn.n; i++)
						if (cplex.getValue(zVar[v][k][i]) > 0.9) {
							//cout << cplex.getValue(zVar[v][k][i]) << "	"	<< zVar[v][k][i].getName() << "	" << endl;
							//no k virtual mapeado em i fisico
							vector[v].vnfs[k].fisico = i;
						}

				for (int kl = 0; kl < vector[v].virtual_links.size(); kl++) {
					vector[v].virtual_links[kl].caminho.clear();
					for (int i = 0; i < sn.n; i++)
						for (int j = 0; j < sn.n; j++)
							if (sn.link[j][i].used == true)
								if (cplex.getValue(xVar[v][kl][i][j]) > 0.9) {
									Caminho caminho;
									caminho.u = i;
									caminho.v = j;
									//cout<<caminho.u<<" "<<caminho.v<<" | ";
									vector[v].virtual_links[kl].caminho.push_back(
											caminho);
								}
				}
				vector[v].mapp_delay = calcula_delayy(vector[v], sn);
			} else
				vector[v].maped = false;
		}

		cout << "\n\nTempo ILP:	"
				<< (((clock() - tInicio) / (CLOCKS_PER_SEC / 1000))) << endl;


		env.end();
		return true;

	} catch (int e) {
		cout << "\n	------->>An exception !!!!!!!!. Exception Nr. " << e
				<< '\n';
		return true;
	}

}

