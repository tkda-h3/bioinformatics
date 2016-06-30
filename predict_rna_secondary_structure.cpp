#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <ctime>
#include <climits>
using namespace std;

#define DEBUG 1
#define NO_PAIR -1

class MBP{
public:
	int val;
	MBP* trace;
	int pair_index;//recursionのk
	int right;//recursionのj
	int left;
	MBP(){
		val = -1;
		trace = NULL;
		pair_index = NO_PAIR;
	}
};

class MBParray{
 public:
	class MBP **mbp;
	int len;
	string str;
	char* result;
	MBParray(string &str){
		len = str.length();
		this->str = str;
		result = new char[len+1];
		for(int i=0; i<len-1; i++)
			result[i] = ' ';
		result[len] = (char)NULL;
		mbp = new MBP*[len];
		for(int i=0; i<len; i++)
			mbp[i] = new MBP[len];
		for(int i=0; i<len; i++)
			for(int j=0; j<len; j++){
				mbp[i][j].left = i;
				mbp[i][j].right = j;
			}
	}
	~MBParray(){
		delete[] result;
		for(int i=0; i<len; i++)
			delete mbp[i];
		delete[] mbp;
	}
	void initialize(){
		for(int i=0; i<len; i++){
			mbp[i][i].val = 0;
			if(i != len-1)//mbp[len-1][len]は存在しない
				mbp[i][i+1].val = 0;//隣同士はペアを組まない問題設定
		}
	}
	
	void update(MBP &parent, MBP &child){
		parent.val = child.val;
		parent.pair_index = NO_PAIR;
		parent.trace = &child;
	}
	void update(MBP &parent, int index, int val){
		parent.val = val;
		parent.pair_index = index;
		parent.trace = NULL;
	}
	//indexの大小でvalidate
	int return_val(int s, int t){
		if(s>t)
			return 0;
		else
			return mbp[s][t].val;
	}
	//kとjがペアを組むかどうか
	bool make_pair(int k, int j){
		string acgu = "ACGU";
		int k_val,j_val;
		for(int i=0; i<(int)acgu.length(); i++){
			if(str[k]==acgu[i])
				k_val = i;
			if(str[j]==acgu[i])
				j_val = i;
		}
		//(a,u)or(c,g)?
		return (k_val+j_val == 3) ? true : false;
	}
	
	void recursion(){
		int cnt=2;
		while(cnt<len){
			for(int i=0; i<len; i++){
				int j=i+cnt;
				if(j >= len) break;
				//処理開始
				if(mbp[i][j].val < mbp[i][j-1].val)
					update(mbp[i][j], mbp[i][j-1]);
				for(int k=i; k<=j-2; k++){//ペアが隣同士は考えない問題設定
					if(make_pair(k,j)){//kとjはペアを組むか
						int val = return_val(i,k-1) + mbp[k+1][j-1].val + 1;
						if(mbp[i][j].val < val)
							update(mbp[i][j], k, val);//j pair is k
					}
				}
				//処理終了
			}
			cnt++;			
		}
		set_result();
	}
	
	//結果を表示
	void set_result(){
		list<MBP*> list;
	  list.push_back(&mbp[0][len-1]);
		while(list.size() > 0){
			MBP* mbp = list.front();
		  list.pop_front();
			if(mbp->pair_index == NO_PAIR){//pairなし
				if(mbp->trace){//mbpが初期条件でないときに再帰
					list.push_front(mbp->trace);
				}
			}else{//pairを組む
				int k = mbp->pair_index;
				int i= mbp->left;
				int j = mbp->right;
				result[k] = '(';
				result[j] = ')';
				if(DEBUG)cout <<"("<<k<<","<<j<<")"<<endl;
				list.push_front(&this->mbp[i][k-1]);
				list.push_front(&this->mbp[k+1][j-1]);
			}
		}
	}	
	void print_result(){
		cout << str << endl;
		cout << result << endl;
	}
	void execute(){
		initialize();
		recursion();
		print_result();
	}
};

//RNA secondary structure prediction
void rna_ssp(){
	string str = "AACGGAACCAACAUGGAUUCAUGCUUCGGCCCUGGUCGCG";	
	MBParray array(str);
	array.execute();
}

int main(){
	rna_ssp();
	return 0;
}
