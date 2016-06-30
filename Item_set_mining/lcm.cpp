#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <climits>
#include <algorithm>
#include <map>
using namespace std;

#define DEBUG 0

//プロトタイプ宣言
class LCM;
//aとbの中身が同じか
bool is_equal(vector<int> &a, vector<int> &b);
//fromの要素の値を全てtoに加える
void all_push_back(vector<int> &from, vector<int> &to);
void print_array(vector<int> &target);
int find_max_from_matrix(vector< vector<int> > &mat);
bool is_exists(vector<int> &array, int target);

class LCM{
public:
	//vectorの２次元
	vector< vector<int> > trsc;//transaction各人の購入物
	int sita;
	int max_item_index;
  LCM(){}
  LCM(int minimal_support){
		sita = minimal_support;
	}
	~LCM(){
		for(int i=0; i<(int)trsc.size(); i++)
			trsc[i].clear();
		trsc.clear();
	}
	void set_mining_test_data(){
		//minimal support
		sita= 1;
		//item set data
		trsc.push_back(std::vector<int>{1,2,5,6,7,9});
		trsc.push_back(std::vector<int>{2,3,4,5});
		trsc.push_back(std::vector<int>{1,2,7,8,9});
		trsc.push_back(std::vector<int>{1,7,9});
		trsc.push_back(std::vector<int>{2,7,9});
		trsc.push_back(std::vector<int>{2});
		max_item_index = find_max_from_matrix(trsc);
	}
	//support(sita)の数を数える
	int compute_support(vector<int>&pattern, vector< vector<int> > &td){
		int n=0;
		for(int i=0; i<(int)td.size(); i++){
			if(is_trsc_include_pattern(pattern,td[i]))
				n++;
			}
		return n;
	}
	bool is_trsc_include_pattern(vector<int>&pattern, vector<int>&transaction){
		if(DEBUG){
			cout << "is_trsc_include_pattern関数内において"<<endl;
			cout << "pattern array" << endl; print_array(pattern);
			cout << "transaction array" << endl; print_array(transaction);
		}
		for(int i=0; i<(int)pattern.size(); i++){
			bool flag=false;
			for(int j=0; j<(int)transaction.size(); j++){
				if(pattern[i] == transaction[j]){
					flag = true; break;
				}
			}
			if(!flag) return false;
		}
		return true;
	}	

	vector<int> intersection(vector<int> &pattern, vector<int> &transaction){
		vector<int> ans;
		for(int i=0; i<(int)pattern.size(); i++){
			for(int j=0; j<(int)transaction.size(); j++){
				if(pattern[i] == transaction[j])
					ans.push_back(pattern[i]);
			}
		}
		return ans;
	}

	vector< vector<int> > return_denotation(vector<int> &closure, vector< vector<int> > &td){
		vector< vector<int> > ans;
		for(int i=0; i<(int)td.size(); i++)
			if(is_trsc_include_pattern(closure, td[i]))
					ans.push_back(td[i]);
		return ans;
	}

	//make closure to avoid duplicated output
	vector<int> compute_closure(vector<int> &pattern, vector< vector<int> > &td){
		vector<int> closure;
		for(int i=0; i<(int)td.size(); i++){
			if(is_trsc_include_pattern(pattern, td[i])){
				if(closure.empty()) all_push_back(td[i],closure);
				else closure = intersection(closure, td[i]);
				if(closure.empty()) return closure;
			}
		}
		return closure;
	}

	bool is_ppc(vector<int> &pattern ,vector<int> &closure, int index){
		vector<int>tmp;
		for(int i=1; i<(int)index; i++)
			tmp.push_back(i);
		vector<int> result = intersection(closure,tmp);
		if(is_equal(pattern,result))
			return true;
		else
			return false;
	}

	void backtrack(vector<int> &pattern, vector<int> &closure, int ci, vector< vector<int> > &td){
		if(DEBUG){cout << "ci == " << ci << endl;}
		if(DEBUG){cout << "pattern == "; print_pattern(pattern,cout);}
		if(DEBUG){cout << "td size == "<<td.size() << endl<<endl;}
		if(td.size() == 1) return;
		for(int i=ci+1; i<=max_item_index; i++){
			vector<int> new_pattern = pattern; 
			new_pattern.push_back(i); 
			//std::sort(new_pattern.begin(), new_pattern.end());
			if(DEBUG){cout<<"new pattern"<<endl; print_array(new_pattern);}
			vector<int> new_closure = compute_closure(new_pattern,td);
			if(DEBUG){cout<<"new closure"<<endl; print_array(new_closure);}
			int support_num = compute_support(new_pattern,td);
			if(is_ppc(pattern,new_closure,i) && support_num >= this->sita){
				vector< vector<int> > new_td = return_denotation(new_closure,td);
				if(DEBUG){cout << "new_td.size() == " << new_td.size()<<endl;}
				if(!is_exists(closure,i)){
						cout << "closure == "; 
						print_pattern(new_closure,cout);
				}
				backtrack(new_pattern,new_closure,i,new_td); 
			}
		}
	}
	
	void print_pattern(vector<int> &pattern,ostream &output){
	  output << "{";
		for(int i=0; i<(int)pattern.size(); i++){
		  output<<pattern[i];
			if(i != (int)pattern.size()-1)
			  output <<",";
		}
	  output << "}"<<endl;
	}

	void recursion(){
		if((int)trsc.size() < sita) return;
		vector<int> pattern;
		vector<int> closure = compute_closure(pattern,trsc);
		cout << "closure == "; print_pattern(closure,cout);
		backtrack(pattern,closure,0,trsc);
	}
	void execute_test(){
		cout << "LCM algorithm test" << endl;
		print_mining_data();
		recursion();
	}	
	void print_mining_data(){
		for(int i=0; i<50; i++) cout << "*"; cout<<endl;
		cout << "------データベースの表示------" << endl;
		for(int i=0; i<(int)trsc.size(); i++){
			cout << "["<<i<<"] == {";
			for(int j=0; j<(int)trsc[i].size(); j++){
				cout << trsc[i][j];
				if(j!=(int)trsc[i].size()-1) cout<< ",";
			}
			cout << "}" << endl;
		}		
		cout << "-----minimal supportの表示---" << endl;
		cout << "θ == " << sita << endl;
		for(int i=0; i<50; i++) cout << "*"; cout<<endl<<endl;		
	}	

	void backtrack(vector<int> &pattern, vector<int> &closure, int ci, vector< vector<int> > &td, multimap<int,vector<int>,greater<int> > &frequent){
		if(td.size() == 1) return;
		for(int i=ci+1; i<=max_item_index; i++){
			vector<int> new_pattern = pattern; 
			new_pattern.push_back(i); 
			vector<int> new_closure = compute_closure(new_pattern,td);
			int support_num = compute_support(new_pattern,td);
			if(is_ppc(pattern,new_closure,i) && support_num >= this->sita){
				vector< vector<int> > new_td = return_denotation(new_closure,td);
				if(!is_exists(closure,i)){
				  frequent.insert(pair<int,vector<int> >(new_td.size(),new_pattern));
				}
				backtrack(new_pattern,new_closure,i,new_td,frequent); 
			}
		}
	}

	void write_top10_for_retail(multimap<int,vector<int>,greater<int> > order, ostream& output){
		if(order.size() < 10){
			output<<"write_to10_for_retail関数内に異常"<<endl;
			exit(1);
		}
		{
			int cnt=0;
			for(multimap<int,vector<int>,greater<int> >::iterator p=order.begin(); cnt<10; p++,cnt++){
				output.width(2);
				output <<cnt+1<<"位 : "<<(*p).first<<"個 ";
				print_pattern((*p).second,output);
			}
		}
	}
	void recursion_for_retail(){
		if((int)trsc.size() < sita) return;
		vector<int> pattern;
		vector<int> closure = compute_closure(pattern,trsc);
		multimap<int,vector<int>,greater<int> > frequent;
		frequent.insert(pair<int,vector<int> >(trsc.size(),pattern));
		backtrack(pattern,closure,0,trsc,frequent);
		write_top10_for_retail(frequent,cout);
		{
			ofstream ost("retail.txt");
			if(!ost){
				cerr<<"retail.txt cannot open"<<endl; 
				return;
			}
			write_top10_for_retail(frequent,ost);
			ost.close();
			cout << "retail.txtにもfrequent patternを出力しました。"<<endl;
		}
	}	
	void execute_test_for_retail(){
		cout << "enumerateing 10 most frequent pattern" << endl;
		print_mining_data();
		recursion_for_retail();
	}	
};

int find_max_from_matrix(vector< vector<int> > &mat){
	int tmp_max = -INT_MAX;
	for(int i=0; i<(int)mat.size(); i++){
		for(int j=0; j<(int)mat[i].size(); j++){
			tmp_max = (tmp_max < mat[i][j]) ? mat[i][j] : tmp_max;
		}
	}
	return tmp_max;
}

//arrayの中にtargetという値が含まれているか否か
bool is_exists(vector<int> &array, int target){
	for(int i=0; i<(int)array.size(); i++){
		if(array[i] == target) return true;
	}
	return false;
}

void print_array(vector<int> &target){
	for(int i=0; i<(int)target.size(); i++){
		cout <<"["<<i<<"] == "<<target[i]<<endl;
	}
}
//fromの要素の値を全てtoに加える
void all_push_back(vector<int> &from, vector<int> &to){
	for(int i=0; i<(int)from.size(); i++)
		to.push_back(from[i]);
}
//aとbの中身が同じか
bool is_equal(vector<int> &a, vector<int> &b){
	if(a.size() != b.size()) return false;
	for(int i=0; i<(int)a.size(); i++)
		if(a[i] != b[i]) return false;
	return true;
}

//item set miningを実行する
void execute_item_set_mining(){
  LCM lcm;
	lcm.set_mining_test_data();
	lcm.execute_test();
	cout << endl<<endl<<endl;
	lcm.execute_test_for_retail();
}

int main(){
	execute_item_set_mining();
	return 0;
}

