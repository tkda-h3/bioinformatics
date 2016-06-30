#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <utility>
#include <map>
#include <set>
#include <string>
using namespace std;

class Alignment{
public:
  int sx,sy,ex,ey,score;
  string id;
  class Alignment *trace;
  Alignment(int start_x, int start_y, int end_x, int end_y, int sc, string name){
    sx = start_x;
    sy = start_y;
    ex = end_x;
    ey = end_y;
    score = sc;
    id = name;
    trace = NULL;
  }
  //半開区間における大小
  bool operator<(Alignment b){
    return (ex <= b.sx && ey <= b.sy);
  }
  bool is_start(int val){
    if(val == sx)
      return true;
    else
      return false;
  }
  //sort_xの先頭がsxの時に連結する場合の関数
  void Alicat(Alignment &pre){
    score += pre.score;
    trace = &pre;
  }
  void print_trace(){
    cout << id << endl;
    Alignment *p = trace;
    while(p){
      cout << p->id << endl;
      p = p->trace;
    }
  }
};

//Debug用
void print_sort_x(multimap<int ,Alignment*> &sort_x);
void print_sort_y(map<int ,Alignment*> &sort_y);

//Debug用
std::ostream &operator<<(std::ostream &stream, Alignment &a){
  stream << "Alignment ID : " << a.id << " score : " << a.score << endl;
  stream << "(" << a.sx << "," << a.sy << ") ~ (" << a.ex << "," << a.ey << ")" << endl; 
  return stream;
}

void set_test_data(set<Alignment*> &allData){
  //(sx,sy,ex,ey,score,id)
  allData.insert(new Alignment( 1, 1, 7, 9,10,"a")); 
  allData.insert(new Alignment( 3,23, 9,32,11,"b")); 
  allData.insert(new Alignment( 7,18,10,24, 5,"c")); 
  allData.insert(new Alignment(14,10,21,19,11,"d")); 
  allData.insert(new Alignment(20, 0,26, 7,11,"e")); 
  allData.insert(new Alignment(18,29,23,36, 5,"f")); 
  allData.insert(new Alignment(25,26,30,32, 7,"g")); 
  allData.insert(new Alignment(29,14,34,19, 5,"h")); 
  allData.insert(new Alignment(32,22,40,36,14,"i")); 
}

void sort_allData(set<Alignment*> &allData, multimap<int, Alignment*> &sort_x){
  for(set<Alignment*>::iterator p = allData.begin(); p != allData.end(); p++){
    sort_x.insert(make_pair((*p)->sx, *p));
    sort_x.insert(make_pair((*p)->ex, *p));
  }
}

void loop_step(multimap<int, Alignment*> &sort_x, map<int, Alignment*> &sort_y){
  for(multimap<int, Alignment*>:: iterator p = sort_x.begin(); 
      !sort_x.empty(); 
      p = sort_x.begin()){
    Alignment *B = p->second;
    if(B->is_start(p->first)){//start_x(1の場合)
      Alignment *tmp = NULL;
      for(map<int, Alignment*>:: iterator q = sort_y.begin(); q != sort_y.end(); q++){
	Alignment *C = q->second;
	if(*C < *B)//演算子オーバーロード
	  tmp = C;
      }
      if(tmp){
	B->Alicat(*tmp);	
      }
    }else{//end_x(2の場合)
      bool ans = true;//(B.ey,B)のYへ追加の是非
      for(map<int, Alignment*>:: iterator q = sort_y.begin(); q != sort_y.end(); q++){
	Alignment *C = q->second;
	if(C->ey <= B->ey && C->score >= B->score){ 
	  ans = false;
	  break;
	}
      }
      if(ans){//(B.ey,B)追加する
	sort_y.insert(make_pair(B->ey,B));
      }
      for(map<int, Alignment*>:: iterator q = sort_y.begin(); q != sort_y.end();){
	Alignment *D = q->second;
	if(B->ey <= D->ey && B->score > D->score){
	  map<int, Alignment*>:: iterator tmp = q++;//erase()の前にiteratorを確保
	  sort_y.erase(tmp);
	}else
	  q++;//forの3つ目の条件式が空欄だから
      }
    }
    sort_x.erase(p);
  }
}

void print_sort_x(multimap<int ,Alignment*> &sort_x){
  cout << "sort_xの表示" << endl;
  for(multimap<int, Alignment*>:: iterator p = sort_x.begin(); p != sort_x.end(); p++){
    if(p->second->is_start(p->first))
      cout << "sxです" << endl;
    else
      cout << "exです" << endl;
    cout << *(p->second) << endl;
  }
}

void print_sort_y(map<int ,Alignment*> &sort_y){
  cout << "sort_yの表示" << endl;
  for(map<int, Alignment*>:: iterator p = sort_y.begin(); p != sort_y.end(); p++){
    cout << *(p->second) << endl;
  }
}

void set_random_test_data(set<Alignment*> &allData){
  srand((unsigned)time(NULL));
  int cnt = 0 ;
  while(rand() % 50000 != 0){
    cnt++;
    int sx = rand() % 500;
    int sy = rand() % 500;
    int add_x = rand() % 30;
    int add_y = rand() % 30;
    int score = rand() % 25 + 5;
    string name = "a";
    allData.insert(new Alignment(sx, sy, sx + add_x,sy + add_y, score, name));
  }
  if(cnt < 5)
    set_test_data(allData);
  else
    cout << "データ数は　" << cnt << endl;
}

void delete_all_new(set<Alignment*> &allData){
  for(set<Alignment*>::iterator p = allData.begin(); p != allData.end(); p++){
    delete *p;
  }
}

//chainingを実行
void execute_chaining(){
  set<Alignment*> allData;
  srand((unsigned)time(NULL));
  if(rand() % 2)
    set_test_data(allData);//テストデータの準備
  else
    set_random_test_data(allData);
  multimap<int ,Alignment*> sort_x;
  sort_allData(allData, sort_x);//AlignmentをXでsort
  //print_sort_x(sort_x);
  map<int, Alignment*> sort_y;
  loop_step(sort_x, sort_y);//繰り返しの処理
  print_sort_y(sort_y);  
  map<int, Alignment*>:: iterator p = sort_y.end();
  p--;
  Alignment *last = p->second;
  cout << "chaing score is " << last->score << endl;
  cout << "chaining ID: " << endl;
  last->print_trace();
  delete_all_new(allData);
}

int main(void){
  clock_t start,end;
  start = clock();
  execute_chaining();
  end = clock();
  cout << "execution time is " << end - start << "[ms]" << endl;
  return 0;
}
