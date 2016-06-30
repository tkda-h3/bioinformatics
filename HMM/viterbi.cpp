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

/* 関数プロトタイプの宣言*/
bool file_error_check(ifstream ist);
template <class X> void new_table(X** &table, int row, int col);
template <class X> void delete_table(X** &table, int row);

/* HMMに必要な初期データ*/
class Read_predata{
public:
  string str;//適用する出力文字列
  int *stri;//strの各文字をintに変換する
  int alpha_num;//アルファベットの数
  char *alpha;//アルファベットの種類
  int state_num;//状態数
  int time;//実質は出力文字数
  long double **trans;//遷移行列
  long double **emit;//出力確率の行列
  long double **log_trans;
  long double **log_emit;
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
    convert_str_to_int();//strをintに変換する
    ist >> state_num;
    getline(ist,tmp);//4 row finish(state number)
    new_table(trans, state_num, state_num);
    new_table(log_trans, state_num, state_num);
    new_table(emit, state_num, alpha_num);
    new_table(log_emit, state_num, alpha_num);    
    for(int i=0; i<state_num; i++){
      for(int j=0; j<state_num; j++){
	ist >> trans[i][j];
	log_trans[i][j] = log(trans[i][j]);
      }
      getline(ist,tmp);
    }//trans row finish
    for(int i=1; i<state_num; i++){//状態0からの出力はないから。
      for(int j=0; j<alpha_num; j++){
	ist >> emit[i][j];
	log_emit[i][j] = log(emit[i][j]);
      }
      getline(ist,tmp);
    }//emit row finish
    ist.close();
  }
  ~Read_predata(){
    delete[] alpha;
    delete[] stri;
    delete_table(trans, state_num);
    delete_table(log_trans, state_num);
    delete_table(emit, state_num);
    delete_table(log_emit, state_num);
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
  }//function finish
};

//sample_RNA.faを読む
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

/* 動的な表の確保 */
template <class X> void new_table(X** &table, int row, int col){
  table = new X*[row];//行を確保
  for(int i=0; i<row; i++)
    table[i] = new X[col];//各行の列を確保
}

/* 動的な表の解放 */
template <class X> void delete_table(X** &table, int row){
  for(int i=0; i<row; i++)
    delete[] table[i];
  delete[] table;
}

//結果の表
class Result_data{
public:
  //  long double **trans;//result of trans
  //  long double **emit;//result of emit
  long double **log_trans;//log of result of trans
  long double **log_emit;//log of result of emit
  int **state_trace;
  int *state;
  int row;
  Result_data(){
  }
  Result_data(int row, int col){
    new_table(log_trans,row,col);
    new_table(log_emit,row,col);
    new_table(state_trace,row,col);
    state = new int[col];
    this->row = row;
  }
  ~Result_data(){
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
};

class Viterbi : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
  Viterbi(Read_predata &data, Result_data &result){
    this->data = &data;
    this->result = &result;
  }
  void initialize(){
    result->log_emit[0][0] = log(1.0);//emit[state][time]
    for(int i=1; i<=data->time; i++)
      result->log_emit[0][i] = log(0.0);
    for(int i=1; i<data->state_num; i++)
      result->log_emit[i][0] = log(0.0);//state "i" ,time 0 の出力記号確率のlog
  }
  void recursion(){
    for(int t=1; t<=data->time; t++){
      for(int s=1; s<data->state_num; s++){
	int max_state = 0;//max_log is derived from max_state 
	long double max_log = log(0.0);
	for(int i=0; i<data->state_num; i++){
	  long double tmp = result->log_emit[i][t-1] + data->log_trans[i][s];
	  if(tmp > max_log){
	    result->log_emit[s][t] = tmp + data->log_emit[s][data->stri[t-1]];//t-1はindexの関係
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

void execute_viterbi(){
  string str;
  read_sample_RNA(str);//sample-RNA.faをstrに読み込む
  Read_predata data("param.txt",str);
  Result_data result(data.state_num, data.time+1);
  Viterbi vit(data, result);
  vit.execute(); 
}

int main(){
  execute_viterbi();
  return 0;
}

