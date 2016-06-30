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

/* �֐��v���g�^�C�v�̐錾*/
template <class X> void new_table(X** &table, int row, int col);
template <class X> void delete_table(X** &table, int row);

/* HMM�ɕK�v�ȏ����f�[�^*/
class Read_predata{
public:
  string str;//�K�p����o�͕�����
  int *stri;//str�̊e������int�ɕϊ�����
  int alpha_num;//�A���t�@�x�b�g�̐�
  char *alpha;//�A���t�@�x�b�g�̎��
  int state_num;//��Ԑ�
  int time;//�����͏o�͕�����
  long double **trans;//�J�ڍs��
  long double **emit;//�o�͊m���̍s��
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
    convert_str_to_int();//str��int�ɕϊ�����
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
    for(int i=1; i<state_num; i++){//���0����̏o�͂͂Ȃ�����B
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

//sample_RNA.fa��ǂ�
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

/* ���I�ȕ\�̊m�� */
template <class X> void new_table(X** &table, int row, int col){
  table = new X*[row];//�s���m��
  for(int i=0; i<row; i++)
    table[i] = new X[col];//�e�s�̗���m��
}

/* ���I�ȕ\�̉�� */
template <class X> void delete_table(X** &table, int row){
  for(int i=0; i<row; i++)
    delete[] table[i];
  delete[] table;
}

// ���ʂ̏o��
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
	    * data->emit[s][data->stri[t-1]];//t-1��index�̊֌W
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
	    * result->emit[i][t+1];//stri[t]��index 1 start��t+1�Ԗڂ̕���
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
			    * data->emit[s][data->stri[0]] /*stri[0]��1 index start��1������*/
			    * result->emit[s][1])
	   << " prob in a log scale." << endl;
    }
    {
      long double tilde_prob = 0;//scale��̊m��
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
  read_sample_RNA(str);//sample-RNA.fa��str�ɓǂݍ���
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

