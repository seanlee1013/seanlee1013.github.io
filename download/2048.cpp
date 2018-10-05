#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>
#define NUM 4

using namespace std;

/*****************************
 * output the Map
 *****************************/

void output(int Map[NUM][NUM]){
    int i,j;
    for(i = 0;i < NUM;++ i){
        for(j = 0;j < NUM;++ j){
            cout << Map[i][j] << '\t';
        }
        cout << endl;
    }
}

/********************************************
 * Transfer every cell to the right direction
 * @para Map:The Map
 *       type: The direction
 ********************************************/
void trans(int Map[NUM][NUM], char type){
    int i,j,k;
    if(type == 'w'){
        for(i = 0;i < NUM - 1;++ i){
            for(j = 0;j < NUM;++ j){
                if(!Map[i][j]) {
                    for(k = i + 1;k < NUM;++ k){
                        if(Map[k][j]){
                            Map[i][j] = Map[k][j];
                            Map[k][j] = 0;
                            k = NUM;
                        }
                    }
                }
            }
        }
    }
    if(type == 'd'){
        for(i = 0;i < NUM;++ i){
            for(j = NUM - 1;j > 0;-- j){
                for(k = j - 1;k >= 0;-- k){
                    if(!Map[i][j]){
                        if(Map[i][k]){
                            Map[i][j] = Map[i][k];
                            Map[i][k] = 0;
                            k = -1;
                        }
                    }
                }
            }
        }
    }
    if(type == 's'){
        for(i = NUM - 1;i > 0;-- i){
            for(j = 0;j < NUM;++ j)
                if(!Map[i][j]) {
                    for(k = i - 1;k >= 0;-- k){
                        if(Map[k][j]){
                            Map[i][j] = Map[k][j];
                            Map[k][j] = 0;
                            k = -1;
                        }
                    }
                }
        }
    }
    if(type == 'a'){
        for(i = 0;i < NUM;++ i){
            for(j = 0;j < NUM - 1;++ j)
                if(!Map[i][j]){
                    for(k = j + 1;k < NUM;++ k){
                        if(Map[i][k]){
                            Map[i][j] = Map[i][k];
                            Map[i][k] = 0;
                            k = NUM;
                        }
                    }
                }
        }
    }
}

/********************************************
 * get new cell
 * @para Map:The Map
 *       type:
 *            1 : check the Map and add a new cell
 *            0 : only check the Map
 ********************************************/
bool init(int Map[NUM][NUM], int type){
    int flag[NUM * NUM];
    memset(flag,0,sizeof(flag));
    int i,j,cnt = 0;
    for(i = 0;i < NUM;++ i){
        for(j = 0;j < NUM;++ j){
            if(!Map[i][j]) flag[cnt ++] = i * 4 + j;
        }
    }
    if(!type) return cnt;
    if(cnt){
        int rnd = rand() % cnt;
        rnd = flag[rnd];
        Map[rnd / 4][rnd % 4] = 2;
        return true;
    }
    return false;
}

/********************************************
 * if a cell is equal to the next one, add up them
 * @para Map:The Map
 *       type: the direction
 ********************************************/

void addup(int Map[NUM][NUM],char type){
    int i,j,dir;
    if(type == 'a'){
        for(i = 0;i < NUM;++ i){
            for(j = 0;j < NUM - 1;++ j){
                if(Map[i][j] == Map[i][j + 1]){
                    Map[i][j] <<= 1;
                    Map[i][j + 1] = 0;
                }
            }
        }
    }else if(type == 'w'){
        for(i = 0;i < NUM - 1;++ i){
            for(j = 0;j < NUM;++ j){
                if(Map[i][j] == Map[i + 1][j]){
                    Map[i][j] <<= 1;
                    Map[i + 1][j] = 0;
                }
            }
        }
    }else if(type == 'd'){
        for(i = NUM - 1;i >= 0;-- i){
            for(j = NUM - 1;j > 0;-- j){
                if(Map[i][j] == Map[i][j - 1]){
                    Map[i][j] <<= 1;
                    Map[i][j - 1] = 0;
                }
            }
        }
    }else if(type == 's'){
        for(i = NUM - 1;i > 0;-- i){
            for(j = NUM - 1;j >= 0;-- j){
                if(Map[i][j] == Map[i - 1][j]){
                    Map[i][j] <<= 1;
                    Map[i - 1][j] = 0;
                }
            }
        }
    }
}

bool execute(int Map[NUM][NUM],char type){
    int i,j,k;
    if(type != 'w' && type != 's' && type != 'a' && type != 'd'){
        if(type == 10) ;
        else printf("Input error\n");
        return true;
    }

    trans(Map,type);

    addup(Map,type);

    trans(Map,type);

    if(init(Map,1)){
        output(Map);
        return true;
    }
    output(Map);
    printf("Game over\n");
    return false;
}

int main(){
    int i,j;
    int Map[NUM][NUM];
    char inputFile[20];
    char input;
    srand((unsigned)time(NULL));
    cout << "Please input the path of the configuration file:" << endl;
    cin >> inputFile;
    ifstream file(inputFile);
    memset(Map,0,NUM * NUM * sizeof(int));
    if(file == NULL){
        Map[NUM - 1][NUM - 1] = 2;
        cout << "Configuration file not found, the map are initialed as the default one" << endl;
        output(Map);
    }else{
        for(i = 0;i < 4;++ i)
            for(j = 0;j < 4;++ j)
                file >> Map[i][j];
        if(!init(Map,0)) {
            output(Map);
            cout << "Game over!" << endl;
            return 0;
        }
        cout << "The map are initialed" << endl;
        output(Map);
    }
    do{
        cin >> input;
    }while(execute(Map,input));

     return 0;
}
