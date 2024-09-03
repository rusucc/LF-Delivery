#include <bits/stdc++.h>
using namespace std;

ifstream fin("graf.in");

bool trav[20]={};
int cost[20]={};
short int A[20][20];
int parent[20]={};
int n;
int path[20];
int path_length=0;

struct node{
    uint8_t rfid[4];
    int id, conn_number=0;
    char letter;
    struct connections{
        node* from;
        node* to;
        char dir=NULL; //l-left, f-forward, r-right
    }conn[20];
    void print(){
        cout<<id<<" "<<letter<<endl;
        for(int i=0;i<conn_number;i++){
            cout<<"From:"<<conn[i].from->letter<<" to:"<<conn[i].to->letter<<endl;
        }
    }
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
void generate_path(char start, char stop, bool print=false){
    int id_start = start-'A';
    int id_stop = stop-'A';
    int curr = id_stop;
    path[path_length++]=id_stop;
    while(parent[curr]!=-1){
        if(print) cout<<char(curr+'A')<<"/"<<curr<<" ";
        curr = parent[curr];
        path[path_length++]=curr;
    }
    path[path_length]=id_start;
    for(int i=0;i<path_length/2;i++) swap(path[i],path[path_length-i-1]);
    if(print) cout<<endl;
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
    string buf;
    fin.get();
    while(getline(fin,buf)){
        //cout<<buf<<endl;
        int nd=int(buf[0]-'A');
        int from=int(buf[1]-'A');
        int to=int(buf[2]-'A');
        int dir=buf[3];
        v[nd].id=nd;
        v[nd].conn[v[nd].conn_number].from=&v[from];
        v[nd].conn[v[nd].conn_number].to=&v[to];
        v[nd].conn[v[nd].conn_number].dir=dir;
        v[nd].conn_number++;
        v[nd].letter=buf[0];
        /*
        cout<<"In nodul "<<n<<"/"<<buf[0]<<" din directia "<<from<<"/"<<buf[1]<<" catre "<<to<<"/"<<buf[2];
        if(dir=='F') cout<<" mergi in fata\n";
        else if(dir=='L') cout<<" se face stanga\n";
        else if(dir=='R') cout<<" se face dreapta\n";
            */
    }
    djikstra('A');
    generate_path('A','I',true);
    for(int i=0;i<path_length;i++){
        cout<<path[i]<<" ";
    }
    cout<<endl<<endl;
    string decisions;
    for(int i=1;i<path_length-1;i++){//luam fiecare nod din path
        //path e int array
        node* current=&v[int(path[i])];
        cout<<current->letter<<endl;
        for(int j=0;j<current->conn_number;j++){
            int fr_id = (current->conn[j]).from->id;
            int to_id = (current->conn[j]).to->id;
            if(fr_id==path[i-1] and to_id==path[i+1]){
                decisions+=(current->conn[j]).dir;
                current->print();
                //cout<<fr_id<<" "<<path[i-1]<<" "<<to_id<<" "<<path[i+1]<<" "<<j<<endl;
                cout<<(current->conn[j]).dir<<" "<<j<<endl<<endl;
            }
        }
    }
    cout<<decisions<<endl;
    return 0;
}
