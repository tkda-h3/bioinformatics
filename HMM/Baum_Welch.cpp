#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdio>
using namespace std;
#define DEBUG (1)

template <class X> void new_table(X** &table, int row, int col);
template <class X> void delete_table(X** &table, int row);
template <class X> void copy_table(X** &target, X** &source, int row, int col);
template <class X> void full_0_table(X** &table, int row, int col);
template <class X> void show_table(X** &table, int row, int col, int srow, int scol, ofstream &ost);
template <class X> void show_table(X** &table, int row, int col, ofstream &ost);
template <class X> void show_table(X** &table, int row, int col, int srow, int scol);
template <class X> void show_table(X** &table, int row, int col);

class Read_predata{
public:
  string str;
  int *stri;
  int alpha_num;
  char *alpha;
  int state_num;
  int time;
  long double **trans;
  long double **emit;
	Read_predata(){}
	Read_predata(string &str, Read_predata &data){
		this->str = str;
		time = str.length();
		alpha_num = 4;
		alpha = new char[alpha_num];
		alpha[0] = 'A'; alpha[1] = 'C'; alpha[2] = 'G'; alpha[3] = 'T';
		stri = new int[time];
		convert_str_to_int();
		state_num = 5;
		new_table(trans, state_num, state_num);
    new_table(emit, state_num, alpha_num);
		copy_table(trans,data.trans,state_num,state_num);
		copy_table(emit,data.emit,state_num,alpha_num);
	}
  Read_predata(char *filename, string &str){
    /*First, set str*/
    this->str = str;
    time = str.length();
    /*Second, set everything else*/
    ifstream ist(filename);
    if(!ist){
      cerr << "Cannot open " << endl;
      exit(1);
    }    
    string tmp;//dummy
    getline(ist,tmp);//1 row finish
    ist >> alpha_num;
    getline(ist,tmp);//2 row finish 
    alpha = new char[alpha_num];
    for(int i=0; i<alpha_num; i++)
      ist >> alpha[i];
    getline(ist,tmp);//3 row finish
    stri = new int[time];
    convert_str_to_int();
    ist >> state_num;
    getline(ist,tmp);//4 row finish(state number)
    new_table(trans, state_num, state_num);
    new_table(emit, state_num, alpha_num);
    for(int i=0; i<state_num; i++){
      for(int j=0; j<state_num; j++){
				ist >> trans[i][j];
      }
      getline(ist,tmp);
    }//trans row finish
    for(int i=1; i<state_num; i++){
      for(int j=0; j<alpha_num; j++){
				ist >> emit[i][j];
      }
      getline(ist,tmp);
    }//emit row finish
    ist.close();
  }
  ~Read_predata(){
    delete[] alpha;
    delete[] stri;
    delete_table(trans, state_num);
    delete_table(emit, state_num);
  }  
  void convert_str_to_int(){
    for(int i=0; i<time; i++){
      for(int j=0; j<alpha_num; j++){
				if(!(str.compare(i,1,(alpha + j),1))){
					stri[i] = j;
					break;
				}
      }
    }
  }
};

void read_sample_RNA(string &str){
  ifstream ist("sample-RNA.fa");
  if(!ist){
    cerr << "Cannot open " << endl;
    exit(1);
  }    
  string tmp;
  getline(ist,tmp);
  while(getline(ist,tmp)){
    str += tmp;
  }
  ist.close();
}

template <class X> void new_table(X** &table, int row, int col){
  table = new X*[row];
  for(int i=0; i<row; i++)
    table[i] = new X[col];
}
template <class X> void delete_table(X** &table, int row){
  for(int i=0; i<row; i++)
    delete[] table[i];
  delete[] table;
}
template <class X> void copy_table(X** &target, X** &source, int row, int col){
  for(int i=0; i<row; i++)
    for(int j=0; j<col; j++)
      target[i][j] = source[i][j];
}
template <class X> void full_0_table(X** &table, int row, int col){
	for(int i=0; i<row; i++)
		for(int j=0; j<col; j++)
			table[i][j] = 0;
}
template <class X> void show_table(X** &table, int row, int col, int srow, int scol ,ofstream &ost){
	for(int i=srow; i<row; i++)
		for(int j=scol; j<col; j++)
			ost << "[" << i << "][" << j << "] == " << table[i][j] << endl;
	ost << endl;
}
template <class X> void show_table(X** &table, int row, int col, ofstream &ost){
	show_table(table, row, col, 0, 0, ost);
}
template <class X> void show_table(X** &table, int row, int col, int srow, int scol){
	show_table(table, row, col, srow, scol, cout);
}
template <class X> void show_table(X** &table, int row, int col){
	show_table(table, row, col, 0, 0, cout);
}
class Result_data{
public:
  long double **trans;//result of trans
  long double **emit;//result of emit
  long double **log_trans;//log of result of trans
  long double **log_emit;//log of result of emit
  int **state_trace;//trace back
  int *state;//result of trace back
  int row;
	Result_data(){}
  Result_data(int row, int col){
    new_table(trans,row,col);
    new_table(emit,row,col);
    new_table(log_trans,row,col);
    new_table(log_emit,row,col);    
    new_table(state_trace,row,col);
    state = new int[col];
    this->row = row;
  }
  ~Result_data(){
    delete_table(trans,row);
    delete_table(emit,row);
    delete_table(log_trans,row);
    delete_table(log_emit,row);
    delete_table(state_trace,row);
    delete[] state;
  }
};

class Algorithm{
public:
  virtual void initialize() = 0;
  virtual void recursion() = 0;
  virtual void execute() = 0;
};

class Viterbi : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
	Viterbi(){}
  Viterbi(Read_predata &data, Result_data &result){
    this->data = &data;
    this->result = &result;
  }
  void initialize(){
    result->log_emit[0][0] = log(1.0);//emit[state][time]
    for(int i=1; i<=data->time; i++)
      result->log_emit[0][i] = log(0.0);
    for(int i=1; i<data->state_num; i++)
      result->log_emit[i][0] = log(0.0);
  }
  void recursion(){
    for(int t=1; t<=data->time; t++){
      for(int s=1; s<data->state_num; s++){
	int max_state = 0;//max_log is derived from max_state 
	long double max_log = log(0.0);
	for(int i=0; i<data->state_num; i++){
	  long double tmp = result->log_emit[i][t-1] + log(data->trans[i][s]);
	  if(tmp > max_log){
	    result->log_emit[s][t] = tmp + log(data->emit[s][data->stri[t-1]]);
	    max_log = tmp;
	    max_state = i;
	  }
	}
	result->state_trace[s][t] = max_state;//trace
      }//s loop
    }//t loop
    int last_state;
    {
      long double max_tmp = log(0.0);
      for(int i=1; i<data->state_num; i++){
				if(result->log_emit[i][data->time] >= max_tmp){
					last_state = i;
					max_tmp = result->log_emit[i][data->time];
				}
      }
    }
    result->state[data->time] = last_state;
    for(int i=data->time; i>0; i--){
      result->state[i-1] = result->state_trace[result->state[i]][i];
    }
  }
  void show_result(){
    ofstream ost("viterbi_result.txt");
    if(!ost){
      cerr << "cannot open" << endl;
      exit(1);
    }
    ost << "show_result_of_viterbi" << endl;
    for(int i=0; i<=data->time; i++)
      ost << result->state[i];    
    ost << endl;
    ost.close();
  }
  void execute(){
    initialize();
    recursion();
    show_result();
  }
};

class Viterbi_for_baum_welch : public Viterbi{
public:
	Viterbi_for_baum_welch() : Viterbi(){}
	Viterbi_for_baum_welch(Read_predata &data, Result_data &result) : Viterbi(data, result){
	}
	void write_result(string filename){
		ofstream ost(filename);
		if(!ost){
			cerr << "cannot open" << endl;
			exit(1);
		}
		ost << "show_result_of_viterbi" << endl;
		ost << data->str << endl;
		for(int i=0; i<=data->time; i++)
			ost << result->state[i];
		ost << endl;
		ost.close();
	}
	void execute(string filename){
		initialize();
		recursion();
		write_result(filename);
	}
};

class Foward : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
  long double *scale,*log_scale;  
	Foward(){}
  Foward(Read_predata &data, Result_data &result){
    this->data = &data;
    this->result = &result;
    scale = new long double[data.time + 1];//scale[0] is dummy
    log_scale = new long double[data.time + 1];//log_scale[0] is dummy
  }
  ~Foward(){
    delete[] scale; 
    delete[] log_scale;
  }
  void initialize(){
    for(int s=0; s<data->state_num; s++){
      for(int t=0; t<=data->time; t++){
				result->emit[s][t] = 0.0;
			}
		}
    result->emit[0][0] = 1.0;
  }	
  void recursion(){
    for(int t=1; t <=data->time; t++){
      long double sp_tmp=0;//sum of prob of tmporary time
      for(int s=1; s<data->state_num; s++){
				for(int i=0; i<data->state_num; i++){
					result->emit[s][t] += result->emit[i][t-1] * data->trans[i][s] 
						* data->emit[s][data->stri[t-1]];
				}
				sp_tmp += result->emit[s][t];
      }
      scale[t] = sp_tmp;
      log_scale[t] = log(sp_tmp);
      //execute scaling
      for(int s=1; s<data->state_num; s++)
				result->emit[s][t] /= scale[t];      
    }
  }
  void show_result(){
    ofstream ost("foward_result.txt");
    if(!ost){
      cerr << "cannot open foward_result.txt" << endl;
      exit(1);
    }
    long double sum_log=0;
    for(int t=1; t<data->time; t++)
      sum_log += log_scale[t]; 
    ost << "------Forward Algorithm----------------" << endl;
    ost << endl << "result is " << sum_log + log(scale[data->time]) << endl << endl;;
  }
  void execute(){
    initialize();
    recursion();
    show_result();
  }
};

class Foward_for_baum_welch : public Foward{
public:
	Foward_for_baum_welch() : Foward(){}
	Foward_for_baum_welch(Read_predata &data, Result_data &result) : Foward(data, result){
	}
	long double get_log_for_baum(int state, int time){
		long double sum_log=0;
		for(int i=1; i<time; i++)
			sum_log += log_scale[i];
		long double ans = sum_log + log(result->emit[state][time]) - log_scale[time];
		return ans;
	}
	long double get_final_answer(){
		long double sum_log=0;
		for(int t=1; t<data->time; t++)
			sum_log += log_scale[t];
		long double ans = sum_log + log(scale[data->time]);
		return ans;
	}
	void execute(){
		initialize();
    recursion();
	}
};

class Backward : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
  long double *scale,*log_scale;  
	Backward(){}
  Backward(Read_predata &data, Result_data &result){
    this->data = &data;
    this->result = &result;
    scale = new long double[data.time];//scale[0] is dummy
    log_scale = new long double[data.time];//log_scale[0] is dummy
  }
  ~Backward(){
    delete[] scale; 
    delete[] log_scale;
  }
	void initialize(){
    for(int s=1; s<data->state_num; s++)
      result->emit[s][data->time] = 1.0;
    for(int s=1; s<data->state_num; s++)
      for(int t=1; t<data->time; t++)
				result->emit[s][t] = 0.0;         
  }
  void recursion(){
    for(int t=data->time-1; t>0; t--){
      long double sp_tmp=0;//sum of prob of tmporary time
      for(int s=1; s<data->state_num; s++){
				for(int i=1; i<data->state_num; i++){
					result->emit[s][t] += data->trans[s][i] * data->emit[i][data->stri[t]]
						* result->emit[i][t+1];
				}
				sp_tmp += result->emit[s][t];
      }
      scale[t] = sp_tmp;
      log_scale[t] = log(scale[t]);
      /*scaling start*/
      for(int s=1; s<data->state_num; s++)
				result->emit[s][t] /= scale[t];
    }
  }  
  void show_result(){
    ofstream ost("backward_result.txt");
    if(!ost){
      cerr << "cannot open foward_result.txt" << endl;
      exit(1);
    }
    long double sum_log=0;
    for(int t=1; t<data->time; t++)
      sum_log += log_scale[t]; 
    ost << "------Backward Algorithm----------------" << endl;
    for(int s=1; s<data->state_num; s++){
      ost << "Finally, state[" << s << "] have " 
					<< sum_log + log(data->trans[0][s] 
													 * data->emit[s][data->stri[0]]
													 * result->emit[s][1])
					<< " prob in a log scale." << endl;
    }
    {
      long double tilde_prob = 0;
      for(int i=1; i<data->state_num; i++)
				tilde_prob += data->trans[0][i] * data->emit[i][data->stri[0]] 
					* result->emit[i][1];
      ost << endl << "result is " << sum_log + log(tilde_prob) << endl << endl;    
    }
  }
  void execute(){
    initialize();
    recursion();
    show_result();
  }
};

class Backward_for_baum_welch : public Backward{
public:
	Backward_for_baum_welch() : Backward(){}
	Backward_for_baum_welch(Read_predata &data, Result_data &result) : Backward(data, result){
	}
	long double get_log_for_baum(int state, int time){
		if(time == data->time)
			return 0.0;
		long double sum_log=0;
		for(int i=time + 1; i<data->time; i++)
			sum_log += log_scale[i];
		long double tilde_prob=0;
		for(int i=1; i<data->state_num; i++){
			tilde_prob += data->trans[state][i] * data->emit[i][data->stri[time]]
				* result->emit[i][time+1];
		}
		long double ans = sum_log + log(tilde_prob); 
		return ans;
	}
	void execute(){
    initialize();
    recursion();
  }
};

class Baum_Welch : public Algorithm{
public:
  Read_predata  *data;
	Read_predata **read;
  Foward_for_baum_welch **fwd;
  Backward_for_baum_welch **bkd;
  int lsize;//learning array number
  long double **trans;
  long double **emit;
  long double **trans_time;
  long double **emit_time;
  Baum_Welch(int snum, Read_predata &data, Read_predata* &read,Foward_for_baum_welch* &fwd, Backward_for_baum_welch* &bkd){
    lsize = snum;
		this->data  = &data;
		this->read = &read;
		this->fwd = &fwd;
		this->bkd = &bkd;
		for(int i=0; i<lsize; i++){
			new_table(trans, data.state_num, data.state_num); 
			new_table(emit, data.state_num, data.alpha_num);
			new_table(trans_time, data.state_num, data.state_num);
			new_table(emit_time, data.state_num, data.alpha_num);
		}	
  }
	~Baum_Welch(){
		delete_table(trans,data->state_num);
		delete_table(emit,data->state_num);
		delete_table(trans_time,data->state_num);
		delete_table(emit_time,data->state_num);
  }	
  void initialize(){
		copy_table(trans,data->trans,data->state_num,data->state_num);
		copy_table(emit,data->emit,data->state_num,data->alpha_num);
  }	
  void recursion(){
		int repeat = 3;
		for(int i=0; i<repeat; i++){
			if(i>0){
				for(int i=0; i<lsize; i++){
					copy_table(read[i]->trans,trans,data->state_num,data->state_num);
					copy_table(read[i]->emit,emit,data->state_num,data->alpha_num);
					fwd[i]->execute();
					bkd[i]->execute();
				}
			}
			full_0_table(trans_time,data->state_num,data->state_num);
			full_0_table(emit_time,data->state_num,data->alpha_num);			
			e_step(); 
			m_step();
		}
  }	

	void e_step(){
		for(int len=0; len<lsize; len++){//strv index
			long double p = fwd[len]->get_final_answer();			
				for(int i=0; i<data->state_num; i++){
					long double b = bkd[len]->get_log_for_baum(0,1);
					trans_time[0][i] += exp(b-p) * trans[0][i] * emit[i][read[len]->stri[0]];
				}			
			for(int t=1; t<read[len]->time; t++){//time
				for(int i=1; i<data->state_num; i++){//trans from i					
					long double f = fwd[len]->get_log_for_baum(i,t);
					for(int j=1; j<data->state_num; j++){//to j
						long double b = bkd[len]->get_log_for_baum(j,t+1);
						trans_time[i][j] += exp(f+b-p) 
							* trans[i][j] * emit[j][read[len]->stri[t-1]];
					}
				}
				for(int i=1; i<data->state_num; i++){
					long double f = fwd[len]->get_log_for_baum(i,t);
					long double b = bkd[len]->get_log_for_baum(i,t);
					emit_time[i][read[len]->stri[t-1]] += exp(f + b - p);
				}
			}
		}//len	
	}
	
	void m_step(){
		{
			long double* sum_trans_time = new long double[data->state_num];
			for(int i=0; i<data->state_num; i++)
				sum_trans_time[i] = 0; 
			for(int i=0; i<data->state_num; i++)
				for(int j=1; j<data->state_num; j++)
					sum_trans_time[i] += trans_time[i][j];
			for(int i=0; i<data->state_num; i++){
				for(int j=1; j<data->state_num; j++){
					trans[i][j] = trans_time[i][j] / sum_trans_time[i];
				}
			}
			delete[] sum_trans_time;
		}
		{
			long double* sum_emit_time = new long double[data->state_num];
			for(int i=1; i<data->state_num; i++)
				sum_emit_time[i] = 0;
			for(int i=1; i<data->state_num; i++)
				for(int j=0; j<data->alpha_num; j++)
					sum_emit_time[i] += emit_time[i][j];
			for(int i=1; i<data->state_num; i++)
				for(int j=0; j<data->alpha_num; j++)
					emit[i][j] = emit_time[i][j] / sum_emit_time[i];
			delete[] sum_emit_time;
		}		
	}
  void show_result(){
		ofstream ost1("baumWelch_trans_result.txt"),ost2("baumWelch_emit_result.txt");
		if(!ost1 || !ost2){
			cerr << "cannot open ost1 or ost2" << endl;
			exit(1);
		}
		show_table(trans, data->state_num, data->state_num, ost1);
		show_table(emit, data->state_num, data->alpha_num, 1, 0, ost2);
		ost1.close();
		ost2.close();
		cout << "write baumWelch_trans_result.txt and baumWelch_emit_result.txt" << endl;
  }
	void write_result(){
		for(int i=0; i<lsize; i++){
			copy_table(data->trans,trans,data->state_num,data->state_num);
			copy_table(data->emit,emit,data->state_num,data->alpha_num);
			Read_predata d(read[i]->str, *data);
			Result_data res(read[i]->state_num, read[i]->time+1);
			Viterbi_for_baum_welch vit(d,res);
			string filename = "Viterbi_for_baum_welch_string" + to_string((long double)i) + ".txt";
			vit.execute(filename);
		}
	}
  void execute(){
    initialize();
    recursion();
    show_result();
		write_result();
  }
};

void read_learning_data(char* filename, vector <string> &strv){
	ifstream ist(filename);
	if(!ist){
		cerr << "cannot open " << filename << endl;
		exit(1);
	}
	string read;
	while(getline(ist,read)){
		if(read[0] != '>'){
			strv[strv.size() - 1] += read;
		}else{
			string tmp = "";
			strv.push_back(tmp);
		}
	}
	ist.close();
}  

void execute_baum_welch(){	
  string str;
  read_sample_RNA(str);
  Read_predata data("param.txt",str);
  vector <string> strv; 
	read_learning_data("trna-seq.txt",strv);
	int snum = strv.size();	
	Read_predata** read = new Read_predata*[snum];
	Result_data** fwd_result = new Result_data*[snum];
	Result_data** bkd_result = new Result_data*[snum];
	Foward_for_baum_welch** fwd = new Foward_for_baum_welch*[snum];
	Backward_for_baum_welch** bkd = new Backward_for_baum_welch*[snum];
	for(int i=0; i<snum; i++){
		read[i] = new Read_predata(strv[i],data);
		fwd_result[i] = new Result_data(read[i]->state_num,read[i]->time+1);
		fwd[i] = new Foward_for_baum_welch(*read[i],*fwd_result[i]);
		fwd[i]->execute();
		bkd_result[i] = new Result_data(read[i]->state_num,read[i]->time+1);
		bkd[i] = new Backward_for_baum_welch(*read[i],*bkd_result[i]);
		bkd[i]->execute();
	}
	Baum_Welch baum(snum,data,*read,*fwd,*bkd);	
	baum.execute();
}

int main(){
  execute_baum_welch();
  return 0;
}

