#include <bits/stdc++.h>
using namespace std;

ifstream fin("graf.in");

bool trav[20]={};
int cost[20]={};
short int A[20][20];
int parent[20]={};
int n;

struct nod{
    uint8_t rfid[4];
    int id;
}v[20];

int djikstra(char start){
    int id_start = start-'A';
    for(int i = 0; i < n; i++){
        cost[i] = 255;
        trav[i] = false;
    }
    cost[id_start]=0;
    trav[id_start]=1;
    parent[id_start]=-1;
    queue<pair<int,int>> p;
    p.push(make_pair(id_start,0));
    while(!p.empty()){
        for(int i=0;i<n;i++){
            if(A[p.front().first][i]!=255){//este vecin
                int cost_inst = p.front().second+A[p.front().first][i];
                if(cost[i]>cost_inst){//daca traseul actual are un costa asociat mai mic decat cel cunoscut, il analizeaza
                    cost[i]=cost_inst;
                    if(trav[i]==0)
                    parent[i]=p.front().first,p.push(make_pair(i,cost_inst));
                }
            }
        }
        trav[p.front().first]=1;
        p.pop();
    }
}
void print_path(char start, char stop){
    int id_start = start-'A';
    int id_stop = stop-'A';
    int curr = id_stop;
    while(parent[curr]!=-1){
        cout<<char(curr+'A')<<" ";
        curr = parent[curr];
    }
}
int main()
{
    fin>>n;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            fin>>A[i][j];
        }
    }
    for(int i=0;i<n;i++){
        for(int j=i;j<n;j++){
            A[j][i]=A[i][j];
        }
    }
    djikstra('A');
    for(int i=0; i<n; i++){
        cout<<char('A'+i)<<" ";
    }
    cout<<endl;
    for(int i=0; i<n; i++){
        cout<<cost[i]<<" ";
    }
    cout<<endl;
    for(int i=0; i<n; i++){
        cout<<parent[i]<<" ";
    }
    cout<<endl;
    for(int i=0; i<n; i++){
        cout<<char(parent[i]+'A')<<" ";
    }
    cout<<endl;
    print_path('A','I');
    return 0;
}
