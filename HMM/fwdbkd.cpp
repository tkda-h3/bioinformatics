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

// 結果の出力
class Result_data{
public:
  long double **trans;//result of trans
  long double **emit;//result of emit
  //  long double **log_trans;//log of result of trans
  //  long double **log_emit;//log of result of emit
  int **state_trace;//trace back
  int *state;//result of trace back
  int row;
  Result_data(int row, int col){
    new_table(trans,row,col);
    new_table(emit,row,col);
    new_table(state_trace,row,col);
    state = new int[col];
    this->row = row;
  }
  ~Result_data(){
    delete_table(trans,row);
    delete_table(emit,row);
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

class Foward : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
  long double *scale,*log_scale;  
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
    for(int s=0; s<data->state_num; s++)
      for(int t=0; t<=data->time; t++)
	result->emit[s][t] = 0.0;
    result->emit[0][0] = 1.0;
  }
  void recursion(){
    for(int t=1; t <=data->time; t++){
      long double sp_tmp=0;//sum of prob of tmporary time
      for(int s=1; s<data->state_num; s++){
	for(int i=0; i<data->state_num; i++){
	  result->emit[s][t] += result->emit[i][t-1] * data->trans[i][s] 
	    * data->emit[s][data->stri[t-1]];//t-1はindexの関係
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
    for(int s=1; s<data->state_num; s++){
      ost << "Finally, state[" << s << "] have " 
	   << sum_log + log(result->emit[s][data->time]) 
	   << " prob in a log scale." << endl;
    }
    ost << endl << "result is " << sum_log + log(scale[data->time]) << endl << endl;;
  }
  void execute(){
    initialize();
    recursion();
    show_result();
  }
};

class Backward : public Algorithm{
public:
  Read_predata *data;
  Result_data *result;
  long double *scale,*log_scale;  
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
	    * result->emit[i][t+1];//stri[t]でindex 1 startのt+1番目の文字
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
			    * data->emit[s][data->stri[0]] /*stri[0]は1 index startの1文字目*/
			    * result->emit[s][1])
	   << " prob in a log scale." << endl;
    }
    {
      long double tilde_prob = 0;//scale後の確率
      for(int i=1; i<data->state_num; i++)
	tilde_prob += data->trans[0][i] * data->emit[i][data->stri[0]] * result->emit[i][1];
      ost << endl << "result is " << sum_log + log(tilde_prob) << endl << endl;    
    }
  }
  void execute(){
    initialize();
    recursion();
    show_result();
  }
};

void execute_fwd_and_bkd(){
  string str;
  read_sample_RNA(str);//sample-RNA.faをstrに読み込む
  Read_predata data("param.txt",str);
  Result_data result(data.state_num, data.time+1);
  /*foward start*/
  Foward fwd(data,result);
  fwd.execute();
  /*backward start*/
  Backward bkd(data,result);
  bkd.execute();
}

int main(){
  execute_fwd_and_bkd();
  return 0;
}

