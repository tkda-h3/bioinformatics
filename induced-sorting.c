#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define NUMBER 33
#define Ltype 0
#define Stype 1
#define LMStype 2//S[[i]がLMSのときtype[i] >= LMStype  LMSの時,文字列sのインデックスが大きい順に2,3,4...
#define EQ 1 //equal
#define NEQ 0 //not equal
#define FREE -1

void print_SA_state(char* str, int *s,int *SA, int length){
  for(int i=0; i<length; i++){
    printf("i=%2d SA[i]=%2d str[SA[i]]: %s\n", i, SA[i], str + SA[i]);
  }
}

void check_order(char *s, int *SA, int length){
  int flag=0;
  for(int i=0; i<length - 1; i++){
    int result = strcmp(s+SA[i], s+SA[i+1]);
    if(result > 0){
      printf("SA[%d]とSA[%d]が逆点している\n", i, i+1);
      flag = 1;
    }
  }  
  if(!flag){
    printf("文字列長%dのSORTING成功\n\n", length);
  }else{
    printf("文字列長%dのSORTING失敗\n\n", length);
  }
}

void random_array(char *str, int number){
  for(int i=0; i<number; i++){
    switch(rand() % 4 + 1){
    case 1: str[i] = 'A'; break; 
    case 2: str[i] = 'C'; break; 
    case 3: str[i] = 'G'; break; 
    case 4: str[i] = 'T'; break;  
    }
  }
  str[number-1] = '$';
  str[number] = '\0';
}

void convert_char_to_int(char *str, int length, int *s){
  for(int i=0; i<length; i++){
    switch(str[i]){
    case '$': s[i] = 0; break; 
    case 'A': s[i] = 1; break; 
    case 'C': s[i] = 2; break; 
    case 'G': s[i] = 3; break; 
    case 'T': s[i] = 4; break; 
    }
  }
}

// return each char start index and last index
void set_each_char_index(int *s, int length, int *start, int *last, int kind){
  int *count = (int *)malloc(sizeof(int)*kind);
  for(int i=0; i<kind; i++)
    count[i] = 0;
  for(int i=0; i<length; i++)
    count[s[i]]++;
  for(int i=0; i<kind; i++)
    count[i] += count[i-1];//各文字の最後のindex + 1
  start[0] = 0;
  for(int i=0; i<kind-1; i++)
    start[i+1] = count[i];   
  for(int i=0; i<kind; i++)
    last[i] = count[i] - 1;
  free(count);
}

int set_type(int *s, int length, int *type){
  int LMS_num=0;
  int LMS_index = LMStype;
  type[length - 1] = LMS_index++;
  LMS_num++;
  for(int i=length-2; 0<=i; i--){
    if(s[i] == s[i+1]){
      type[i] = type[i+1];
    }else{
      if(s[i] < s[i+1])
	type[i] = Stype;//1
      else{
	type[i] = Ltype;//0
	if(type[i+1] == Stype){
	  /*LMSの時文字列sのインデックスが大きい順に2,3,4...*/
	  type[i+1] = LMS_index++;
	  LMS_num++;
	}
      }
    }
  }
  return LMS_num;
}

//step1
void set_LMS_to_SA(int *s,int *SA, int *type,int *EQflag, int length, int *start, int *last, int kind){
  int *count=(int *)malloc(sizeof(int)*kind);
  for(int i=0; i<kind; i++)
    count[i] = last[i];
  for(int i=0; i<length; i++){//iはLMS_stringのi文字目という意味
    if(type[i] >= LMStype){
      //各文字グループのlastはNEQとしたいので後述
      SA[count[s[i]]] = i;
      EQflag[count[s[i]]] = EQ;
      count[s[i]]--;
    }
  }
  for(int i=0; i<kind; i++)
    EQflag[last[i]] = NEQ;//前述した修正を行う
  free(count);
}

//step2
void sort_Ltype_suffix(int *s, int *SA, int *type, int *EQflag, int length, int *start, int *last, int kind){
  int *count=(int *)malloc(sizeof(int)*kind);
  int *leftEQ=(int *)malloc(sizeof(int)*kind);
  for(int i=0; i<kind; i++){
    count[i] = start[i];//count[i]は文字iのhatの位置
    leftEQ[i] = 0;
  }
  int EQcheck = 0;//@を左から移動させた時,EQが連続で出現中
  int EQid = 1;//等号groupID
  for(int i=0; i<length; i++){//iは@を指す
    if(SA[i] != FREE){
      int tmp = SA[i] - 1;
      if(tmp >= 0){//SA[i] == 00の対応策
	if(type[tmp] == Ltype){//sort	  
	  if(EQcheck && leftEQ[s[tmp]] == EQid){
	    //hatの左は "="
	    EQflag[count[s[tmp]] - 1] = EQ; 
	  }
	  SA[count[s[tmp]]] = tmp;//hat位置に代入	 
	  EQflag[count[s[tmp]]] = NEQ;//hatの右はとりあえずNEQ
	  count[s[tmp]]++;
	}
      }//if(tmp >= 0) end
      if(EQflag[i] == EQ){
	EQcheck = 1;
	leftEQ[s[tmp]] = 1;//s[tmp]-groupのhatの左側の記号は等号の可能性あり
	leftEQ[s[tmp]] = EQid;//等号groupの区別
      }else{
	EQcheck = 0;
	leftEQ[s[tmp]] = 0;
	EQid++;//連続する等号を抜けたので等号groupIDを変更
      }      
    }         
  }//for
  free(count);
  free(leftEQ);
}

//step3
void sort_Stype_suffix(int *s, int *SA, int *type, int *EQflag, int length, int *start, int *last, int kind){
  int *count=(int *)malloc(sizeof(int)*kind);
  int *rightEQ=(int *)malloc(sizeof(int)*kind);
  for(int i=0; i<kind; i++){
    count[i] = last[i];//count[i]は文字iのhatの位置
    rightEQ[i] = 0;
  }
  int EQcheck = 0;//@を右から移動させた時EQが連続で出現中flag
  int EQid = 1;//連続する等号groupにIDを振り区別する
  for(int i = length - 1; i>=1; i--){//iは@を指す
    if(SA[i] != FREE){
      int tmp = SA[i] - 1;
      if(tmp >= 0){
	if(type[tmp] == Stype || type[tmp] >= LMStype){//sort
	  if(EQcheck && rightEQ[s[tmp]] == EQid){
	    //hatの右に "=" を入れなおす
	    EQflag[count[s[tmp]]] = EQ;
	  }
	  SA[count[s[tmp]]] = tmp;//hatの位置に代入
	  EQflag[count[s[tmp]] - 1] = NEQ;//hatの左はとりあえずNEQ
	  count[s[tmp]]--;
	}
      }
      if(EQflag[i-1] == EQ){
	EQcheck = 1;
	rightEQ[s[tmp]] = EQid;
      }else{
	EQcheck = 1;
	rightEQ[s[tmp]] = 0;
	EQid++;
      }
    }//if(SA != FREE ...)
  }
  free(count);
  free(rightEQ);
}

int make_LMS_string(int *SA, int length, int *type,int *LMS_string, int *old_s_index, int *EQflag, int LMS_num){
  int str_num = 0;//各LMS部分文字列に割り当てる数字
  int NEQflag=0;//NEQでない
  int max_type = LMStype + LMS_num - 1;//set_typeの挙動参照
  for(int i=0; i<length; i++){
    if(type[SA[i]] >= LMStype){
      if(NEQflag == 1)//NEQである
	str_num++;
      LMS_string[max_type - type[SA[i]]] = str_num;
      //SのSA[i]文字目がLMS_stringのcount_index文字目
      old_s_index[max_type - type[SA[i]]] = SA[i];
      NEQflag = 0;//reset
    }
    if(EQflag[i] == NEQ)//
      NEQflag = 1;
  }
  //str_num == 文字の種類数 - 1
  return str_num + 1;
}

//再起後のLMSの順序決定済step1
void set_LMS_to_SA_by_LMS_order(int *s, int *SA, int *type, int *EQflag, int length, int *start, int *last, int kind, int *LMS_order, int LMS_num){
  int *count=(int *)malloc(sizeof(int)*kind);
  for(int i=0; i<kind; i++)
    count[i] = last[i];
  for(int i=LMS_num-1; i>=0; i--){
    int tmp = LMS_order[i];
    SA[count[s[tmp]]--] = tmp;
  }
  free(count);
}

void construct_SA_not_including_NEQ(int *s, int *SA, int *type, int length, int *start, int *last, int kind, int *EQflag, int *LMS_order, int LMS_num){
  for(int i=0; i<length; i++){
    SA[i] = FREE;
    EQflag[i] = NEQ;
  }
  set_LMS_to_SA_by_LMS_order(s, SA, type, EQflag, length, start, last, kind, LMS_order, LMS_num);//step1
  sort_Ltype_suffix(s, SA, type, EQflag, length, start, last, kind);//step2
  sort_Stype_suffix(s, SA, type, EQflag, length, start, last, kind);//step3  
}

int *construct_SA_by_IS_sub(int *s, int length, int kind){//kindは文字の種類
  int *start, *last;
  start = (int *)malloc(sizeof(int) * kind);
  last = (int *)malloc(sizeof(int) * kind);
  set_each_char_index(s, length, start, last,kind);//set start and last
  int *type=(int *)malloc(sizeof(int)*length);
  int LMS_num = set_type(s, length, type);//L-type or S-type or LMS を調べ,LMSの個数を返す 
  int *EQflag = (int *)malloc(sizeof(int)*length);//その文字の右側の記号を示す
  int *SA=(int *)malloc(sizeof(int)*length);
  for(int i=0; i<length; i++)
    EQflag[i] = SA[i] = FREE;//まだ不明
  for(int i=0; i<kind; i++)
    EQflag[last[i]] = NEQ;//文字グループ境界は設定
  set_LMS_to_SA(s, SA, type, EQflag, length, start, last, kind);//step1
  sort_Ltype_suffix(s, SA, type, EQflag, length, start, last, kind);//step2
  sort_Stype_suffix(s, SA, type, EQflag, length, start, last, kind);//step3
  int *new_parent_s_index = (int *)malloc(sizeof(int)*LMS_num);
  //new_parent_s_indexはLMS_stringに代入したのが元の文字列の何番目に当たるか
  int *LMS_string = (int *)malloc(sizeof(int)*LMS_num);
  int LMS_kind = make_LMS_string(SA, length, type, LMS_string, new_parent_s_index, EQflag, LMS_num);  
  if(length == LMS_kind){//sort済になるまで
    free(start);
    free(last);
    free(type);
    free(EQflag);
    free(new_parent_s_index);
    free(LMS_string);
    return SA;
  }
  int *LMS_SA = construct_SA_by_IS_sub(LMS_string, LMS_num, LMS_kind);
  int *LMS_to_parent_SA_index = (int *)malloc(sizeof(int)*LMS_num);//LMS_SAの各順位に位置しているのはoldでいう何文字目なのか
  for(int i=0; i<LMS_num; i++){
    LMS_to_parent_SA_index[i] = new_parent_s_index[LMS_SA[i]];//new_parent_s_index[x]はLMS_string[x]が元々はpasrent_string
  }
  //ここに再帰的に到達した時old_SA_indexにはsのLMSがsortされて返っている
  construct_SA_not_including_NEQ(s, SA, type, length, start, last, kind, EQflag, LMS_to_parent_SA_index, LMS_num);
  free(start);
  free(last);
  free(type);
  free(EQflag);
  free(new_parent_s_index);
  free(LMS_string);
  free(LMS_SA);
  free(LMS_to_parent_SA_index);
  return SA;
}

void construct_SA_by_IS(char *str, int length){
  int *s=(int *)malloc(sizeof(int)*length);
  convert_char_to_int(str, length, s);
  int *SA = construct_SA_by_IS_sub(s, length, 5);//SAにセット
  if(length == NUMBER){
    print_SA_state(str, s, SA, length);
    check_order(str, SA, length);
  }
  free(s);
  free(SA);
}

int main(void){
  //sadakaneと比較
  char *compare = (char *)malloc(sizeof(char)*(NUMBER+1));
  random_array(compare,NUMBER);
  construct_SA_by_IS(compare,NUMBER);  
  free(compare);

  int max_length = (int)pow(10,8);
  for(int i=100; i<=max_length; i *= 10){
    char *str=(char *)malloc(sizeof(char)*(i+1));
    random_array(str,i);
    clock_t start, end;
    start = clock();
    construct_SA_by_IS(str,i);
    end = clock();
    free(str);    
    printf("文字列長%dのSA構築実行時間: %d[ms]\n", i, (int)(end - start));
  }
  return 0;
}
