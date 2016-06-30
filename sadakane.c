#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define NUMBER 50

void print_suffix_array(char *s, int *SA, int *ISA, int length){
    for(int i=0; i<length; i++){
      printf("i = %3d SA[i] = %3d ISA[SA[i]] = %3d %s\n", i, SA[i], ISA[SA[i]], s + SA[i]);
  }
}

/* ACGT配列の作成,最後は$ */
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
  printf("作成した文字列\n%s\n", str);
}

int selectRandomPosition(int start, int end){
  int i = (rand() % (end - start + 1)) + start;
  return i;
}

void swap(int* v, int x, int y){
  int tmp = v[x];
  v[x] = v[y];
  v[y] = tmp;
}

int partition(int left, int right, int *ISA, int *SA, int skip){
  int random = selectRandomPosition(left, right);
  swap(SA, random, right);
  int pivot = ISA[SA[right] + skip];
  int i = left-1;
  int j = right;
  while(1){
    for(i++; ISA[SA[i] + skip] < pivot; i++)
      ;
    for(j--; pivot < ISA[SA[j] + skip] && i<j; j--)
      ;
    if(i >= j)
      break;
    swap(SA, i, j);
  }
  swap(SA, i, right);
  return i;//sufixでのランキング
}

void TernarySplitQuickSort(int left, int right, int *ISA, int *SA, int skip){
  if(left < right){
    int i = partition(left, right, ISA, SA, skip);
    int pivot = ISA[SA[i] + skip];
    int terminal = i;
    for(int j=left; j<terminal; j++){
      if(ISA[SA[j] + skip] == pivot)
	swap(SA, j--, --terminal);
    } 
    if(left < terminal-1){
      TernarySplitQuickSort(left, terminal-1, ISA, SA, skip);
    }
    terminal = i;
    for(int j=right; j>terminal; j--){
      if(ISA[SA[j] + skip] == pivot)
	swap(SA, j++, ++terminal);
    }
    if(terminal+1 < right){
      TernarySplitQuickSort(terminal+1, right, ISA, SA, skip);
    }
  }
}

/*その文字の最後のindexの直後をreturn*/
void set_count(char *str, int left, int right, int *count, int *SA, int skip){
  for(int i=left; i<=right; i++){
    switch(skip==0 ? str[i] : str[SA[i] + skip]){
    /* skip == 0 の時まだ SAが構築されていないから */
    case '$': count[0]++; break;
    case 'A': count[1]++; break;
    case 'C': count[2]++; break;
    case 'G': count[3]++; break;
    case 'T': count[4]++; break;
    }
  }
  count[0] += left;
  for(int i=1; i<5; i++){
    count[i] = count[i-1] + count[i];
  }
  // count[5] = right + 1;
}
void set_ISA(char *str, int left, int right, int *ISA, int *SA, int *count, int skip){
  /* ここを埋める　*/
  if(left == right)
    return;
  int last_index_of_each_char[5];
  for(int i=0; i<5; i++){
    last_index_of_each_char[i] = count[i] - 1;
  }
  for(int i=right; i>=left; i--){
    //SA[i]はSの何文字目かを表す
    int index;
    switch(skip==0 ? str[i] : str[SA[i] + skip]){
    case '$': index=0; break;
    case 'A': index=1; break;
    case 'C': index=2; break;
    case 'G': index=3; break;
    case 'T': index=4; break;
    }
    SA[--count[index]] = i;    
    ISA[i] = last_index_of_each_char[index];
    /* 文字列中のi文字目のISAを返す*/
  }
}

void update_ISA(int left, int right, int *ISA, int *SA, int skip){
  if(left == right)
    return;
  //ISA[SA[left]] ~ ISA[SA[right]]は全てrightになっているので更新する
  int *update = (int *)malloc(sizeof(int)*(right - left + 1));
  //ISAを随時更新するとアルゴリズムが破綻するので最後にまとめ更新する
  /* ISA[SA[left + i]] = update[i]; に対応する(i = 0 ~ right-left) */
  update[right - left] = ISA[SA[right]];
  int j = right;//leftでなければなんでも良い
  for(int i=right; i>left; i--){
    if(j == left)//forの二重ループ脱出のためにこの位置に用意
      break;
    for(j=i-1; j>=left; j--){
      if(ISA[SA[i]+skip] == ISA[SA[j]+skip])
	update[j - left] = i;
      else{//jの位置からはISA = j;としたい
	update[j - left] = j;
	i = j+1;//forのi--を考慮した微調整
	break;
      }
      if(j == left)
	break;
    }
  }
  for(int i=left; i<=right; i++){
    ISA[SA[i]] = update[i - left];
  }
  free(update);
}

void sadakane_sub(int left, int right, int *ISA, int *SA, int skip){
  /*left~rightの範囲でcountする*/
  TernarySplitQuickSort(left, right, ISA, SA, skip);    
  update_ISA(left, right, ISA, SA, skip);
}

void sadakane(char* s, int length){
  int *SA = (int *)malloc(sizeof(int)*length);
  int *ISA = (int *)malloc(sizeof(int)*length);
  int count[5] = {0,0,0,0,0};
  set_count(s, 0, length - 1, count, SA, 0);
  set_ISA(s, 0, length - 1, ISA, SA, count, 0);
  for(int skip=1; skip<length; skip = skip * 2){
    /*これからISAが同じ値の範囲を求める*/
    for(int i=0; i<length-1; i++){
      int tmp = ISA[SA[i]];
      int j;
      for(j=i+1; j<length; j++){
	if(tmp != ISA[SA[j]])
	  break;
      }
      if(j-1 > i)//left == rightの時sortする必要なし
	sadakane_sub(i, j-1, ISA, SA, skip);	
      i = j-1;//forでi++になるから調整
    }
  }
  printf("Suffix Array の構築結果\n");
  print_suffix_array(s, SA, ISA, length);
  free(SA);
  free(ISA);
}

int main(){
  char *s = (char *)malloc(sizeof(char)*NUMBER);
  random_array(s, NUMBER);
  sadakane(s, NUMBER);
  free(s); 
  return 0;
}
