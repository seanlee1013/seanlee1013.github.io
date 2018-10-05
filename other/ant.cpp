#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


const int iAntCount=20;//ant numbers
const int TrainOutCount=10;//��ȥ���г���
const int TrainInCount=12;//�������г���
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
    int Tdc;//����ʱ�䣬����ʱ��
    int Tjb;//����ʱ��,����ʱ��
    int dir[4];//�ĸ�����ĳ�
};

struct GInfo
{
    TrainInfo Arrive[TrainInCount + 1];//�����г�
    TrainInfo Go[TrainOutCount + 1];//�����г�
    double m_dTrial[TrainInCount][TrainInCount];//��Ϣ�ش洢
}Map;
class ant
{
private:
	double prob[TrainInCount];//ÿ�����е�ѡ�����
	int m_iJTrain;//�Ѿ������˶��ٳ�
	int m_iFTrain;//�Ѿ�����˶��ٳ�������
	int allowed[TrainInCount];//�Ƿ����ѡ�����
	int dir[4];//abcd�ĸ�����ֱ�Ĵ泵 0-a 1-b 2-c 3-d
	int timestamp;//�������н������е�ʱ���

	int ChooseNextCity();//ѡ����һ������
	bool finishNow();//�жϵ�ǰ�ĳ��������ǲ��ǿ�������������һ����
	void nextLevel();//������һ�������������ж�

	bool usable(int i);//�ж�ĳ��ѡ���ǲ�������

	void UpdateProb();//����ÿֻ������һ��ѡ����еĸ���
public:
	bool success;
	double m_dLength;
    int total;//�ܹ����ȥ�ĳ���

	int visited[TrainInCount+1];//��¼ÿֻ�����н������е��г��������

	ant();
	void addcity(int city);//������һ�������г�
	void Clear();//��������
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

bool ant::finishNow(){//��һ���Ĳ��ĳ��������
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

void ant::nextLevel(){//������һ���㼶
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

	if(finishNow()) nextLevel();//����ǲ��ǿ��Ե���һ����
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
            p /= sum_Trial;//ʹ�ù�ʽ
            prob[i] = p;
            temp += p;
            c ++;
		}
	}

    for (i = 0;i < avilibale[m_iFTrain];++ i){
		if(allowed[i]){
			if (temp == 0)
				prob[i] = (double)1.0 / (double)c;//����ܵ���0����ô��ƽ����ֵ
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
    for(i = 0;i < avilibale[m_iFTrain];++ i){//ת�ַ�ѡ����һ������
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

	double m_dShortest;//�����е����Ž����������
	ant ants[iAntCount];//����������е�����
	int besttour[TrainInCount + 1];//��¼��������õĽ������
	int res[TrainInCount][TrainOutCount][4];//��¼��̬�����Ľ��
	int In_last[TrainInCount][4];//�ھ�̬���������У�ÿ�����ﳵ����ʣ�೵����ʱ�洢
    int tempTrain[TrainOutCount][4];//��ʱ�洢��ǰ�����ĳ������ж��پ�����

	void UpdateTrial();//������Ϣ��
	void Sta_output(int out);//��̬����������
	void GetAnt();//�������ϵĳ�ʼλ��
	void StartSearch();//��ʼ������������
	bool putin(int in,int out);//��̬���������м����µĵ��ﳵ��
	void StableDiv();//��̬����
	bool addup(int now,int out);//ÿ�ο����µĳ���������ʱ���ȿ��ǵ�ǰʣ�೵���ǲ��ǿ�������
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
				ants[j].move();//������һ�����ƶ�

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
			Map.m_dTrial[i][j] *= (1-rou);//��Ϣ�ص���ɢ��
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

void project::StableDiv(){//��̬��������
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

