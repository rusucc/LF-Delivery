#include <queue>
#include <string>

bool trav[20] = {};
int cost[20] = {};
short int A[20][20] = {
  { 255, 2, 2, 255, 255, 255, 255, 255, 255 },
  { 2, 255, 1, 255, 255, 255, 255, 255, 255 },
  { 2, 1, 255, 1, 255, 255, 255, 255, 255 },
  { 255, 255, 1, 255, 1, 2, 255, 255, 255 },
  { 255, 255, 255, 1, 255, 2, 255, 1, 255 },
  { 255, 255, 255, 2, 2, 255, 255, 255, 255 },
  { 255, 255, 255, 255, 255, 255, 255, 1, 2 },
  { 255, 255, 255, 255, 1, 255, 1, 255, 2 },
  { 255, 255, 255, 255, 255, 255, 2, 2, 255 }
};
int parent[20] = {};
int n = 9;
int path[20] = {};
char decisions[50] = {};
int path_length = 0;
int connectionListLength = 34;

struct node {
  uint8_t rfid[4];
  int id, conn_number = 0;
  char letter;
  struct connections {
    node* from = nullptr;
    node* to = nullptr;
    char dir = NULL;  //l-left, f-forward, r-right
  } conn[20];
} v[20];

// current, from, to, direction
char connectionList[40][4] = {
  { 'A', 'B', 'C', 'F' },
  { 'A', 'C', 'B', 'F' },
  { 'B', 'A', 'C', 'L' },
  { 'B', 'C', 'A', 'R' },
  { 'C', 'A', 'D', 'L' },
  { 'C', 'B', 'D', 'F' },
  { 'C', 'B', 'A', 'L' },
  { 'C', 'D', 'A', 'R' },
  { 'C', 'A', 'B', 'R' },
  { 'C', 'D', 'B', 'F' },
  { 'D', 'C', 'F', 'L' },
  { 'D', 'E', 'F', 'L' },
  { 'D', 'F', 'E', 'L' },
  { 'D', 'C', 'E', 'F' },
  { 'D', 'E', 'C', 'F' },
  { 'D', 'F', 'C', 'R' },
  { 'F', 'E', 'D', 'F' },
  { 'F', 'D', 'E', 'F' },
  { 'E', 'H', 'F', 'R' },
  { 'E', 'D', 'F', 'L' },
  { 'E', 'F', 'D', 'R' },
  { 'E', 'H', 'D', 'F' },
  { 'E', 'F', 'H', 'L' },
  { 'E', 'D', 'H', 'F' },
  { 'H', 'E', 'I', 'L' },
  { 'H', 'G', 'I', 'R' },
  { 'H', 'E', 'G', 'F' },
  { 'H', 'I', 'G', 'L' },
  { 'H', 'I', 'E', 'R' },
  { 'H', 'G', 'E', 'F' },
  { 'G', 'I', 'H', 'R' },
  { 'G', 'H', 'I', 'L' },
  { 'I', 'H', 'G', 'F' },
  { 'I', 'G', 'H', 'F' },
};

short p[200][2];

void djikstra(char start);
void generate_path(char start, char stop);
void connectionArrayToStruct();
void generate_decisions();
void setup();
void loop() {
  Serial.println(decisions);
  delay(3000);
}
void connectionArrayToStruct() {
  for (int i = 0; i < connectionListLength; i++) {
    int nd = int(connectionList[i][0] - 'A');
    int from = int(connectionList[i][1] - 'A');
    int to = int(connectionList[i][2] - 'A');
    int dir = connectionList[i][3];
    v[nd].id = nd;
    v[nd].conn[v[nd].conn_number].from = &v[from];
    v[nd].conn[v[nd].conn_number].to = &v[to];
    v[nd].conn[v[nd].conn_number].dir = dir;
    v[nd].conn_number++;
    v[nd].letter = connectionList[i][0];
  }
}
void djikstra(char start) {
  int id_start = int(start - 'A');
  for (int i = 0; i < n; i++) {
    cost[i] = 255;
    trav[i] = false;
  }
  cost[id_start] = 0;
  trav[id_start] = 1;
  parent[id_start] = -1;
  int p_start_index=0;
  int p_end_index=1;
  
  p[0][0] = id_start; 
  p[0][1] = 0;
  while (p_start_index<=p_end_index) {
    delay(1);
    for (int i = 0; i < n; i++) {
      int A_buff = A[(p[p_start_index][0])][i];
      if (A_buff != 255) {  //este vecin
        int cost_inst = p[p_start_index][1] + A_buff;
        if (cost[i] > cost_inst) {  //daca traseul actual are un costa asociat mai mic decat cel cunoscut, il analizeaza
          cost[i] = cost_inst;
          if (trav[i] == 0){
            parent[i] = p[p_start_index][0];
            p[p_end_index][0] = i;
            p[p_end_index][1] = cost_inst;
            p_end_index++;
            Serial.printf("Added element <%d,%d> to queue\n",i,cost_inst);
          }
        }
      }
    }
    trav[p[p_start_index][0]] = 1;
    p_start_index++;
    if(p_start_index>p_end_index) {
      Serial.printf("No more elements\n");
    }
    Serial.printf("Start %d, stop: %d \n",p_start_index,p_end_index);
  }
}
void generate_path(char start, char stop) {
  int id_start = start - 'A';
  int id_stop = stop - 'A';
  int curr = id_stop;
  path[path_length++] = id_stop;
  while (parent[curr] != -1) {
    curr = parent[curr];
    path[path_length++] = curr;
  }
  path[path_length] = id_start;
}
void generate_decisions() {
  for (int i = 1; i < path_length - 1; i++) {  //luam fiecare nod din path
    //path e int array
    node* current = &v[int(path[i])];
    for (int j = 0; j < current->conn_number; j++) {
      int fr_id = (current->conn[j]).from->id;
      int to_id = (current->conn[j]).to->id;
      if (fr_id == path[i - 1] and to_id == path[i + 1]) {
        char buf[2];
        buf[0] = (current->conn[j]).dir;
        buf[1] = '\0';
        strcat(decisions, buf);
      }
    }
  }
}
void setup() {
  Serial.begin(9600);
  connectionArrayToStruct();
  djikstra('A');
  Serial.println("A si iesit din djikstra?");
  generate_path('A','G');
  generate_decisions();
  Serial.println(decisions);
}
