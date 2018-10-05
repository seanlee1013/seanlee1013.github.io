#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


const int iAntCount=20;//ant numbers
const int TrainOutCount=10;//出去的列车数
const int TrainInCount=12;//进来的列车数
const int iItCount=200;
const double rou = 0.5;
int maxtotal;
int avilibale[TrainOutCount];
bool mustfull[TrainOutCount];


double  rnd(int low,int uper){
	double p=(rand()/(double)RAND_MAX)*((uper)-(low))+(low);
	return (p);
};

int rnd(int uper){
	return (rand()%uper);
};

struct TrainInfo{
    int Tdc;//到达时间，出发时间
    int Tjb;//解体时间,编组时间
    int dir[4];//四个方向的车
};

struct GInfo
{
    TrainInfo Arrive[TrainInCount + 1];//到达列车
    TrainInfo Go[TrainOutCount + 1];//出发列车
    double m_dTrial[TrainInCount][TrainInCount];//信息素存储
}Map;
class ant
{
private:
	double prob[TrainInCount];//每个城市的选择概率
	int m_iJTrain;//已经解体了多少车
	int m_iFTrain;//已经完成了多少车的配置
	int allowed[TrainInCount];//是否可以选择城市
	int dir[4];//abcd四个方向分别的存车 0-a 1-b 2-c 3-d
	int timestamp;//在蚂蚁行进过程中的时间戳

	int ChooseNextCity();//选择下一个城市
	bool finishNow();//判断当前的出发车辆是不是可以满足走入下一步了
	void nextLevel();//进入下一个出发车辆的判定

	bool usable(int i);//判断某个选择是不是能用

	void UpdateProb();//更新每只蚂蚁下一次选择城市的概率
public:
	bool success;
	double m_dLength;
    int total;//总共配进去的车数

	int visited[TrainInCount+1];//记录每只蚂蚁行进过程中的列车解体过程

	ant();
	void addcity(int city);//加入下一个解体列车
	void Clear();//重置蚂蚁
	void move();

};


ant::ant(){
	m_dLength = 0;
	m_iJTrain = 0;
	m_iFTrain = 0;
	success = 1;
	timestamp = 0;
	total = 0;
    memset(dir,0,sizeof(dir));
	int i;
	for(i = 0;i < TrainInCount;++ i){
		prob[i]=0.5;
		allowed[i]=1;
	}
}

bool ant::finishNow(){//这一级的拆解的车辆完成了
	if(timestamp > Map.Go[m_iFTrain].Tdc - Map.Go[m_iFTrain].Tjb){
        if(mustfull[m_iFTrain]) {
            success = 0;
            return false;
        }else return true;
	}
    int i;
    for(i = 0;i < 4;++ i) {
        if(dir[i] < Map.Go[m_iFTrain].dir[i]) return false;
    }
    return true;
}

void ant::nextLevel(){//进入下一个层级
    int i;
    for(i = 0;i < 4;++ i) {
        dir[i] -= Map.Go[m_iFTrain].dir[i];
        if(dir[i] < 0){
            total += dir[i] + Map.Go[m_iFTrain].dir[i];
            dir[i] = 0;
        }else total += Map.Go[m_iFTrain].dir[i];
    }
    m_iFTrain ++;
}

void ant::addcity(int city){
    int i;
	visited[m_iJTrain] = city;
	for(i = 0;i < 4;++ i) dir[i] += Map.Arrive[city].dir[i];
	m_iJTrain ++;
	allowed[city] = 0;

	if(timestamp < Map.Arrive[city].Tdc){
        timestamp = Map.Arrive[city].Tdc + Map.Arrive[city].Tjb;
	}else timestamp += Map.Arrive[city].Tjb;

	if(finishNow()) nextLevel();//检查是不是可以到下一步了
}

void ant::UpdateProb(){
	int i;
	double temp = 0;
	double sum_Trial = 0;
	double sita = 0.5;
	int curCity = visited[m_iJTrain-1];
	int c = 0;
	for (i = 0;i < avilibale[m_iFTrain];++ i){
		if(allowed[i]){
            sum_Trial += Map.m_dTrial[curCity][i];
		}
	}

    for (i = 0;i < avilibale[m_iFTrain];++ i){
		if(allowed[i]){
            double p = Map.m_dTrial[curCity][i];
            p /= sum_Trial;//使用公式
            prob[i] = p;
            temp += p;
            c ++;
		}
	}

    for (i = 0;i < avilibale[m_iFTrain];++ i){
		if(allowed[i]){
			if (temp == 0)
				prob[i] = (double)1.0 / (double)c;//如果总的是0，那么就平均赋值
        }
		else
			prob[i] = 0;
	}
}


int ant::ChooseNextCity(){
	int i,j;
	double temp;
	double mRate = rnd(0,1);

	UpdateProb();

    double mSelect=0;
    for(i = 0;i < avilibale[m_iFTrain];++ i){//转轮法选择下一个城市
        mSelect += prob[i];
        if(mSelect >= mRate) {j = i;break;}
    }
	return j;
}

void ant::move(){
	int i,j;
	if(m_iFTrain >= TrainOutCount) return ;
	if(!success) return ;
	j=ChooseNextCity();
	addcity(j);
}

void ant::Clear(){
	m_dLength = 0;
	int i;
	for(i = 0;i < TrainInCount;++ i){
		prob[i] = 0;
		allowed[i] = 1;
	}
	for(i = 0;i < 4; ++ i) dir[i] = 0;
	i = visited[0];
	m_iFTrain = 0;
	m_iJTrain = 0;
	timestamp = 0;
	total = 0;
	success = 1;
	addcity(i);
}

class project{
	void initmap();

public:

	double m_dShortest;//工程中的最优解决配流总数
	ant ants[iAntCount];//工程里边所有的蚂蚁
	int besttour[TrainInCount + 1];//记录工程中最好的解体策略
	int res[TrainInCount][TrainOutCount][4];//记录静态配流的结果
	int In_last[TrainInCount][4];//在静态配流过程中，每个到达车辆的剩余车辆临时存储
    int tempTrain[TrainOutCount][4];//临时存储当前出发的车辆还有多少就满了

	void UpdateTrial();//更新信息素
	void Sta_output(int out);//静态配流结果输出
	void GetAnt();//分配蚂蚁的初始位置
	void StartSearch();//开始搜索的主函数
	bool putin(int in,int out);//静态配流过程中加入新的到达车辆
	void StableDiv();//静态配流
	bool addup(int now,int out);//每次考虑新的出发车辆的时候，先考虑当前剩余车辆是不是可以满足
	project();
};


project::project(){
	int num;
	int i,j;

	initmap();
	memset(res,0,sizeof(res));

	m_dShortest = -1;


	ifstream in("input.txt");

	in >> num;

	for (i = 0;i < num;++ i){
		in >> Map.Arrive[i].Tdc >> Map.Arrive[i].Tjb >> Map.Arrive[i].dir[0]
		>> Map.Arrive[i].dir[1] >> Map.Arrive[i].dir[2] >> Map.Arrive[i].dir[3];
	}

	in >> num;
	maxtotal = 0;

	for(i = 0;i < num;++ i){
		in >> Map.Go[i].Tdc >> Map.Go[i].Tjb >> Map.Go[i].dir[0]
		>> Map.Go[i].dir[1] >> Map.Go[i].dir[2] >> Map.Go[i].dir[3];
		maxtotal += Map.Go[i].dir[0] + Map.Go[i].dir[1] + Map.Go[i].dir[2] + Map.Go[i].dir[3];
	}

    j = 0;
    memset(avilibale,0,sizeof(avilibale));
	for(i = 0;i < TrainOutCount;++ i){
        for(;j < TrainInCount;++ j){
            if(Map.Go[i].Tdc - Map.Go[i].Tjb < Map.Arrive[j].Tdc + Map.Arrive[j].Tjb) {
                avilibale[i] = j;
                break;
            }
        }
        if(j == TrainInCount) avilibale[i] = TrainInCount;
	}

}

void project::initmap(){
	int i;
	int j;
	for(i = 0;i < TrainOutCount;++ i) mustfull[i] = 1;
	for(i=0;i<TrainInCount;i++)
		for (j=0;j<TrainInCount;j++){
			Map.m_dTrial[i][j]=1.0/(double)TrainInCount;
		}
}

void project::GetAnt(){
	int i = 0;
	int city;
	//srand( (unsigned)time(NULL) + rand());
	for( i = 0;i < iAntCount;++ i){
        ants[i].addcity(0);
	}
}

void project::StartSearch(){
	int It = 0;
	int i,j;
	double temp;
	int temptour[TrainInCount + 1];
	while(It < iItCount){
		for(i = 0; i < TrainInCount - 1;++ i)
			for(j = 0;j < iAntCount;++ j)
				ants[j].move();//决定下一步的移动

		for(j = 0;j < iAntCount;++ j){
			ants[j].visited[TrainInCount]=ants[j].visited[0];
		}

		//find out the best solution of the step and put it into temp
		int t;
		temp = -1 ;
		for (t = 0;t < TrainInCount+1;++ t)
			temptour[t]=ants[0].visited[t];
		for(j = 0;j < iAntCount;++ j){
            if(!ants[j].success) continue;
			if(temp < ants[j].total){
				temp = ants[j].total;
				for ( t = 0; t < TrainInCount + 1;t ++)
					temptour[t] = ants[j].visited[t];
			}
		}

        if(temp != -1){
            cout << "temp = " << temp ;

        for ( int t=0;t<TrainInCount;t++)
            printf(" %d ",temptour[t]);
        }
        cout << endl;

		if(temp > m_dShortest){
			m_dShortest = temp;
			printf("%d : %f\n",It,temp);
			for ( t = 0;t < TrainInCount + 1;t ++)
				besttour[t] = temptour[t];
		}

		UpdateTrial();

		for(j=0;j<iAntCount;j++)
			ants[j].Clear();

		It ++;
	}
	printf("The most number of trains is : %f\n",m_dShortest);

	for ( int t=0;t<TrainInCount;t++)
		printf(" %d ",besttour[t]);
    cout << endl << "maxtotal = " << maxtotal << endl;

}

void project::UpdateTrial(){
    int i;
	int j;
	double max_total = -1;
	double sum_total = 0;
	double avg_total;

	for (i=0;i<TrainInCount;i++)
	{
		for (j=0;j<TrainInCount;j++)
		{
			Map.m_dTrial[i][j] *= (1-rou);//信息素淡化散发
		}
	}

    for(i = 0;i < iAntCount;++ i){
        if(ants[i].success){
            if(max_total < ants[i].total) max_total = ants[i].total;
            sum_total += ants[i].total;
        }
    }

    avg_total = sum_total / iAntCount;

    if(avg_total == sum_total) return ;

	for(i = 0;i < iAntCount;++ i){
        if(!ants[i].success) continue;
		for (j = 0;j < TrainInCount;++ j){
			Map.m_dTrial[ants[i].visited[j]][ants[i].visited[j + 1]] += 1 - ((double)ants[i].total - sum_total) / (avg_total - sum_total);
			Map.m_dTrial[ants[i].visited[j + 1]][ants[i].visited[j]] += 1 - ((double)ants[i].total - sum_total) / (avg_total - sum_total);
		}
	}
}

bool project::putin(int in,int out){
    int i;
    for(i = 0;i < 4;++ i){
        if(In_last[in][i] > tempTrain[out][i]){
            In_last[in][i] -= tempTrain[out][i];
            res[in][out][i] = tempTrain[out][i];
            tempTrain[out][i] = 0;
        }else{
            tempTrain[out][i] -= In_last[in][i];
            res[in][out][i] = In_last[in][i];
            In_last[in][i] = 0;
        }
    }
    for(i = 0;i < 4;++ i)
        if(tempTrain[out][i]) return false;

    return true;
}

bool project::addup(int now,int out){
    int i;
    for(i = 0;i <= now;++ i){
        if(putin(besttour[i],out)) return true;
    }
    return false;
}

void project::StableDiv(){//静态配流部分
    memset(tempTrain,0,sizeof(tempTrain));
    int timestamp = 0;
    int i,j;
    for(i = 0;i < TrainOutCount;++ i){
        for(j = 0;j < 4;++ j){
            tempTrain[i][j] = Map.Go[i].dir[j];
        }
    }
    for(i = 0;i < TrainInCount;++ i)
        for(j = 0;j < 4;++ j)
            In_last[i][j] = Map.Arrive[i].dir[j];

    i = 0;
    for(j = 0;j < TrainOutCount;++ j){
        if(j) Sta_output(j - 1);
        if(addup(i,j)){
            continue;
        }
        else{
            if(timestamp > Map.Go[j].Tdc - Map.Go[j].Tjb){
                continue;
            }
        }
        for(;i < TrainInCount;){
            ++ i;
            if(timestamp < Map.Arrive[besttour[i]].Tdc){
                timestamp = Map.Arrive[besttour[i]].Tdc + Map.Arrive[besttour[i]].Tjb;
            }else timestamp += Map.Arrive[besttour[i]].Tjb;

            if(timestamp > Map.Go[j].Tdc - Map.Go[j].Tjb){
                break;
            }else{
                if(putin(besttour[i],j))
                    break;
            }
        }
    }
    Sta_output(TrainOutCount - 1);


}

void project::Sta_output(int out){
    int i,j;
    int temp;
    cout << out + 1 << " :" << endl;
    for(i = 0;i < TrainInCount;++ i){
        temp = 0;
        for(j = 0;j < 4;++ j) temp += res[i][out][j];
        if(temp)
            cout << "    No." << i << "  a:" << res[i][out][0]
            << "    b:" << res[i][out][1] << "  c:" << res[i][out][2]
            << "    d:" << res[i][out][3] << endl;
    }

    cout << endl;
}

int main()
{
	project Train;
	Train.GetAnt();
	Train.StartSearch();
	Train.StableDiv();
	char c;
	scanf("%c",&c);
	return 0;
}

