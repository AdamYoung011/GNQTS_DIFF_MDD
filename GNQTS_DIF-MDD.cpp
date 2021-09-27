#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <math.h>
#include "stock_cal.h"
#include <limits>
#include <stdlib.h>
using namespace std;
namespace fs = std::filesystem;

using namespace std;
#define stock_number 30
#define generation 1
#define particle_number 10
#define rotate_angle 0.0004
#define rotate_angle_up 0.0008
#define rotate_angle_down 0.0002
#define test_times 2
#define test_period_flag 1
#define short_selling_flag 0

double initial_investment = 10000000;
int big = 0, small = 0;
int day = 0;
int first = 0, last = 0;

//當代最佳
vector <double> probability, trend;
vector <int> index[particle_number];
vector <int> result[particle_number];
vector <int> stock_choose_number;
vector <double> money[particle_number], expected_return, risk;
vector <double>  remain_money[particle_number], each_money[particle_number];
vector <int> stock_buy_number[particle_number];

//所有代數中最佳
vector <int> result_global_max(stock_number, 0);
double trend_global_max = 0, expected_return_global_max = 0, risk_global_max = 0;
int gen_global_max = 0;
vector <double>  remain_money_global_max, each_money_global_max, money_global_max;
vector <int> stock_buy_number_global_max, index_global_max;
int stock_choose_number_global_max = 0;

//全部實驗中最佳
vector <double>  remain_money_max, each_money_max, money_max;
vector <int> stock_buy_number_max, index_max;
int stock_choose_number_max = 0, gen_max = 0;
double trend_max = 0, expected_return_max = 0, risk_max = 0;
vector <int> result_max;

//pf
double profit_factor = 0.0;
double positive_return = 0.0, negative_return = 0.0;

//MDD
double max_drop_down = 0.0;
double max_drop_down_ratio = 0.0;
double mdd_sum = 0.0;


//初始化機率陣列(全部皆為0.5)
void initial_probability()
{
	for (int i = 0; i < stock_number; i++)
	{
		probability.push_back(0.5);
	}
}

//random probability進行比較
void measure()
{
	double a = 0;

	for (int i = 0; i < particle_number; i++)
	{
		int choose_number_count = 0;
		for (int j = 0; j < stock_number; j++)
		{
			a = rand() / 32767.0;
			if (a < probability[j])
			{
				result[i].push_back(1);
				choose_number_count++;
				index[i].push_back(j);
			}
			else
			{
				result[i].push_back(0);
			}
		}
		stock_choose_number.push_back(choose_number_count);
	}
}

//計算fitness
void fitness_cal(int day, vector <string> name, vector <double> d, int gen)
{
	vector <double>  stock_price;
	double divide_remain_money;
	int divide_money;
	string str;

	int time_ch = 0;
	int times = day + 1;
	double option_return = 0.0;

	for (int i = 0; i < particle_number; i++)
	{
		if (stock_choose_number[i] == 0)
		{
			expected_return.push_back(0);
			risk.push_back(0);
			trend.push_back(0);
			time_ch = 0;
			continue;
		}

		for (int j = 0; j < day; j++)
		{
			if (time_ch == 0)
			{
				divide_money = int(initial_investment / (double)stock_choose_number[i]);

				for (int h = 0; h < stock_choose_number[i]; h++)
				{
					stock_price.push_back(d[index[i][h]]);
					stock_buy_number[i].push_back(double(divide_money) / stock_price[h]);
					remain_money[i].push_back(divide_money - stock_price[h] * stock_buy_number[i][h]);
					each_money[i].push_back(divide_money);
				}
				divide_remain_money = initial_investment - (double)divide_money * stock_choose_number[i];

				money[i].push_back(initial_investment);
			}
			else
			{
				double tmp = 0;
				for (int h = 0; h < stock_choose_number[i]; h++)
				{
					each_money[i].push_back(d[index[i][h] + name.size() * time_ch] * stock_buy_number[i][h] + remain_money[i][h]);
					tmp = tmp + each_money[i][h + j * stock_choose_number[i]];
					if (h == stock_choose_number[i] - 1)
					{
						tmp = tmp + divide_remain_money;
					}
				}
				money[i].push_back(tmp);
				tmp = 0;
			}
			stock_price.clear();
			time_ch++;
		}

		expected_return.push_back(expected_return_cal(money[i], day + 1, initial_investment));
		risk.push_back(risk_cal(expected_return[i], times, initial_investment, money[i]));

		if (short_selling_flag)
		{
			option_return = -expected_return[i];
		}
		else
		{
			option_return = expected_return[i];
		}

		trend.push_back(trend_ratio_cal(option_return, risk[i]));
		time_ch = 0;
	}

}


//找fitness最大及最小
void find_max_min(int gen, int day)
{

	for (int i = 0; i < particle_number; i++)
	{
		if (i == 0)
		{
			big = i;
			small = i;
		}
		else
		{
			if (trend[i] > trend[big])
			{
				big = i;
			}
			else if (trend[i] < trend[small])
			{
				small = i;
			}
		}
	}
	if (trend[big] > trend_global_max)
	{
		trend_global_max = trend[big];
		expected_return_global_max = expected_return[big];
		risk_global_max = risk[big];
		gen_global_max = gen;
		stock_choose_number_global_max = stock_choose_number[big];

		each_money_global_max.clear();
		for (int j = 0; j < day; j++)
		{
			for (int k = 0; k < stock_choose_number_global_max; k++)
			{
				each_money_global_max.push_back(each_money[big][k + j * stock_choose_number_global_max]);
			}
		}

		remain_money_global_max.clear();
		stock_buy_number_global_max.clear();
		index_global_max.clear();
		money_global_max.clear();
		result_global_max.clear();
		for (int k = 0; k < stock_number; k++)
		{
			result_global_max.push_back(result[big][k]);
		}
		for (int k = 0; k < stock_choose_number_global_max; k++)
		{
			remain_money_global_max.push_back(remain_money[big][k]);
			stock_buy_number_global_max.push_back(stock_buy_number[big][k]);
			index_global_max.push_back(index[big][k]);
		}
		for (int j = 0; j < day; j++)
		{
			money_global_max.push_back(money[big][j]);
		}
	}
}

void not_gate()
{
	for (int i = 0; i < stock_number; i++)
	{
		if (result_global_max[i] - result[small][i] == 1 && probability[i] < 0.5)
		{
			probability[i] = 1 - probability[i];
		}
		else if (result_global_max[i] - result[small][i] == -1 && probability[i] > 0.5)
		{
			probability[i] = 1 - probability[i];
		}
	}
}

//更新機率
void update(int test_time, int read_time, int gen)
{
	for (int i = 0; i < stock_number; i++)
	{
		//rotate_angle = rotate_angle_up - 2 * abs(probability[i] - 0.5) * (rotate_angle_up - rotate_angle_down);  //ANGQTS
		if (result_global_max[i] - result[small][i] == 1)
		{
			probability[i] = probability[i] + rotate_angle;
		}
		else if (result_global_max[i] - result[small][i] == -1)
		{
			probability[i] = probability[i] - rotate_angle;
		}
		//rotate_angle = 0.0;
	}
}

void output_each_slide_data(vector <string> name, vector <double> d, int day, int best_test_time, int best_count, string str)
{
	ofstream output("./2001_01_04-2003_01_14_DJIA/fund/long/train/Y2Q_new/fund_" + str);
	output << fixed << setprecision(20);

	if (trend_max <= 0)
	{
		//output << ",";
		trend_max = 0;
		expected_return_max = 0;
		risk_max = 0;
		money_max.clear();
		each_money_max.clear();
		for (int i = 0; i < day; i++)
		{
			money_max.push_back(initial_investment);
			each_money_max.push_back(initial_investment);
		}
		best_test_time = 0;
		gen_max = 0;
		best_count = 0;
	}

	output << "代數," << generation << endl;
	output << "粒子數," << particle_number << endl;
	output << "旋轉角度上界," << rotate_angle_up << endl;
	output << "旋轉角度下界," << rotate_angle_down << endl;
	output << "旋轉角度," << rotate_angle << endl;
	output << "實驗次數," << test_times << endl << endl;
	output << "初始資金," << initial_investment << endl;
	output << "最後資金," << money_max[day - 1] << endl;
	output << "真實報酬," << money_max[day - 1] - initial_investment << endl << endl;
	output << "預期報酬," << expected_return_max << endl;
	output << "風險," << risk_max << endl;
	output << "起點值," << trend_max << endl;
	output << "找到最佳解世代," << gen_max << endl;
	output << "找到最佳解實驗#," << best_test_time << endl;
	output << "找到最佳解次數," << best_count << endl << endl;
	output << "Number of chosen," << stock_choose_number_max << endl;
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << name[index_max[p]];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "Stock#,";
	if (stock_choose_number_max == 0)
	{
		output << "There is no stock be chosen." << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << name[index_max[p]];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "張數,";
	if (stock_choose_number_max == 0)
	{
		output << 0 << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << stock_buy_number_max[p];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "分配資金,";
	if (stock_choose_number_max == 0)
	{
		output << 0 << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << initial_investment / stock_choose_number_max;
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "剩餘資金,";
	if (stock_choose_number_max == 0)
	{
		output << initial_investment << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << remain_money_max[p];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	for (int i = 0; i < day; i++)
	{
		output << "FS(" << i + 1 << "),";
		if (stock_choose_number_max > 1)
		{
			for (int p = 0; p < stock_choose_number_max; p++)
			{
				output << each_money_max[p + i * stock_choose_number_max];
				output << ",";
			}
			output << money_max[i] << endl;
		}
		else
		{
			output << each_money_max[i];
			output << ",";
			output << money_max[i] << endl;
		}

	}
	output.close();
}

void output_all_slide_data(vector <string> name, vector <double> d, int day, int best_test_time, int best_count, string str)
{
	ofstream output("./2001_01_04-2003_01_14_DJIA/long/train/Y2Q_newtrain.csv", ios_base::app);
	output << fixed << setprecision(20);
	output << str << ",";

	output << stock_choose_number_max << ",";
	for (int i = 0; i < stock_choose_number_max; i++)
	{
		output << name[index_max[i]];

		if (i == stock_choose_number_max - 1)
		{
			output << ",";
		}
		else
		{
			output << " ";
		}
	}
	if (trend_max <= 0)
	{
		output << ",";
		trend_max = 0;
		expected_return_max = 0;
		risk_max = 0;
		best_test_time = 0;
		gen_max = 0;
		best_count = 0;
	}
	output << "起點值," << trend_max << ",";
	output << "預期報酬," << expected_return_max << ",";
	output << "風險," << risk_max << ",";
	output << "最佳解實驗#," << best_test_time << ",";
	output << "最佳解世代數," << gen_max << ",";
	output << "最佳解出現次數," << best_count << ",";
	output << endl;
	output.close();
}


void main_function(vector <string>& name, vector <double>& d, int& day, int test_time, int& times, int read_time)
{
	initial_probability();
	//ofstream output("test_particle_stock_50_2010_01.csv", ios_base::app);
	//output << fixed << setprecision(15);
	for (int i = 0; i < generation; i++)
	{
		measure();
		fitness_cal(day, name, d, i);
		find_max_min(i + 1, day);
		not_gate();
		update(test_time, read_time, i);

		for (int h = 0; h < particle_number; h++)
		{
			result[h].clear();
			index[h].clear();
			money[h].clear();
			stock_buy_number[h].clear();
			remain_money[h].clear();
			each_money[h].clear();
		}
		stock_choose_number.clear();
		expected_return.clear();
		risk.clear();
		trend.clear();
	}
}

void pf_cal()
{
	double m = 0.0;
	double real_return = 0.0;

	m = (money[0][money[0].size() - 1] - money[0][0]) / double(day - 1);
	if (m > 0)
	{
		positive_return += m;
	}
	else
	{
		negative_return += abs(m);
	}
}

void MDD_cal()
{
	double drop = 0.0;

	for (int i = 0; i < day; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			drop = (money[0][j] - money[0][i]) / money[0][j];
			if (drop > max_drop_down)
			{
				first = j;
				last = i;
				max_drop_down = drop;
				max_drop_down_ratio = drop;
			}
		}
	}
	mdd_sum += max_drop_down_ratio;
}

void test_cal(int day, vector <string> name, vector <double> d, double test_initial_investment)
{
	vector <double>  stock_price;
	double divide_remain_money;
	int divide_money;
	string str;

	int time_ch = 0;
	int times = day + 1;

	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < day; j++)
		{
			if (time_ch == 0)
			{
				divide_money = int(test_initial_investment / (double)stock_choose_number_max);

				for (int h = 0; h < stock_choose_number_max; h++)
				{
					stock_price.push_back(d[index_max[h]]);
					stock_buy_number[i].push_back(double(divide_money) / stock_price[h]);
					remain_money[i].push_back(divide_money - stock_price[h] * stock_buy_number[i][h]);
					each_money[i].push_back(divide_money);
				}
				divide_remain_money = test_initial_investment - (double)divide_money * stock_choose_number_max;

				money[i].push_back(test_initial_investment);
			}
			else
			{
				double tmp = 0;
				for (int h = 0; h < stock_choose_number_max; h++)
				{
					each_money[i].push_back(d[index_max[h] + name.size() * time_ch] * stock_buy_number[i][h] + remain_money[i][h]);
					tmp = tmp + each_money[i][h + j * stock_choose_number_max];
					if (h == stock_choose_number_max - 1)
					{
						tmp = tmp + divide_remain_money;
					}
				}
				money[i].push_back(tmp);
				tmp = 0;
			}
			stock_price.clear();
			time_ch++;
		}
		expected_return.push_back(expected_return_cal(money[i], day + 1, test_initial_investment));
		risk.push_back(risk_cal(expected_return[i], times, test_initial_investment, money[i]));
		trend.push_back(trend_ratio_cal(expected_return[i], risk[i]));
		time_ch = 0;
	}
	//pf_cal();
	MDD_cal();
}

void output_test_each_slide_data(vector <string> name, vector <double> d, int day, string str, double test_initial_investment)
{
	ofstream output("./2001_01_04-2003_01_14_DJIA/fund/long/test/Y2Q/fund_" + str);
	output << fixed << setprecision(20);

	output << "代數," << generation << endl;
	output << "粒子數," << particle_number << endl;
	output << "旋轉角度," << rotate_angle << endl;
	output << "實驗次數," << test_times << endl << endl;

	output << "初始資金," << test_initial_investment << endl;
	output << "最後資金," << money[0][money[0].size() - 1] << endl;
	output << "真實報酬," << money[0][money[0].size() - 1] - test_initial_investment << endl << endl;
	output << "預期報酬," << expected_return[0] << endl;
	output << "風險," << risk[0] << endl;
	output << "起點值," << trend[0] << endl;
	output << "Number of chosen," << stock_choose_number_max << endl;
	output << "MDD," << max_drop_down << ",first" << first << ",last" << last << endl;
	output << "MDD_Ratio," << max_drop_down_ratio << endl;
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << name[index_max[p]];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "Stock#,";
	if (stock_choose_number_max == 0)
	{
		output << "There is no stock be chosen." << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << name[index_max[p]];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "張數,";
	if (stock_choose_number_max == 0)
	{
		output << 0 << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << stock_buy_number[0][p];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "分配資金,";
	if (stock_choose_number_max == 0)
	{
		output << 0 << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << test_initial_investment / stock_choose_number_max;
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	output << "剩餘資金,";
	if (stock_choose_number_max == 0)
	{
		output << test_initial_investment << endl;
	}
	for (int p = 0; p < stock_choose_number_max; p++)
	{
		output << remain_money[0][p];
		if (p == stock_choose_number_max - 1)
		{
			output << endl;
		}
		else
		{
			output << ",";
		}
	}
	for (int i = 0; i < day; i++)
	{
		output << "FS(" << i + 1 << "),";
		if (stock_choose_number_max > 1)
		{
			for (int p = 0; p < stock_choose_number_max; p++)
			{
				output << each_money[0][p + i * stock_choose_number_max];
				output << ",";
			}
			output << money[0][i] << endl;
		}
		else
		{
			output << each_money[0][i];
			output << ",";
			output << money[0][i] << endl;
		}
	}
	output.close();
}

void test_output(vector <string> name, vector <double> d, int day, string str, double test_initial_investment)
{
	ofstream output("./2001_01_04-2003_01_14_DJIA/long/test/Y2Q_test.csv", ios_base::app);
	output << fixed << setprecision(20);
	output << str << ",";

	output << stock_choose_number_max << ",";
	for (int i = 0; i < stock_choose_number_max; i++)
	{
		output << name[index_max[i]];

		if (i == stock_choose_number_max - 1)
		{
			output << ",";
		}
		else
		{
			output << " ";
		}
	}
	if (stock_choose_number_max == 0)
	{
		output << ",";
	}
	output << "起點值," << trend[0] << ",";
	output << "預期報酬," << expected_return[0] << ",";
	output << "風險," << risk[0] << ",";
	output << "MDD," << max_drop_down << ",";
	output << "MDD percentage," << max_drop_down_ratio << ",";
	output << endl;
	output.close();
}

//全域最佳解(10000代)清除 不含result
void global_max_clear()
{
	trend_global_max = 0;
	expected_return_global_max = 0;
	risk_global_max = 0;
	gen_global_max = 0;
	remain_money_global_max.clear();
	each_money_global_max.clear();
	money_global_max.clear();
	stock_buy_number_global_max.clear();
	index_global_max.clear();
	stock_choose_number_global_max = 0;
}

//區域最佳解(10顆粒子)清除 不含result
void local_clear()
{
	for (int g = 0; g < particle_number; g++)
	{
		index[g].clear();
		result[g].clear();
		money[g].clear();
		remain_money[g].clear();
		each_money[g].clear();
		stock_buy_number[g].clear();
	}
	expected_return.clear();
	risk.clear();
	trend.clear();
}

//實驗全域最佳解(50次實驗)清除 不含result
void test_global_max_clear()
{
	remain_money_max.clear();
	each_money_max.clear();
	money_max.clear();
	stock_buy_number_max.clear();
	index_max.clear();
	stock_choose_number_max = 0;
	gen_max = 0;
	expected_return_max = 0;
	risk_max = 0;
	trend_max = 0;
}


int main()
{
	vector <string> train_name, test_name;
	vector <double> train_d, test_d;
	vector <filesystem::path> train_file, test_file;
	int line = 0;
	int best_test_time = 0;
	int best_count = 0;
	int count = 0;
	int read_time = 0;
	double test_initial_investment = 0;
	test_initial_investment = initial_investment;
	srand(114);
	//ofstream output("train_Gbest__Portfolio_GNQTS_10000代_10粒子_0.0004_實驗50_Y2Q.csv", ios_base::app);

	for (auto& p : fs::directory_iterator("./2001_01_04-2003_01_14/Y2Q/train"))
	{
		train_file.push_back(p.path());
	}
	for (auto& p : fs::directory_iterator("./2001_01_04-2003_01_14/Y2Q/test"))
	{
		test_file.push_back(p.path());
	}

	for (int i = 0; i < train_file.size(); i++)
	{
		read_time++;
		ifstream train_in(train_file[i]);
		string str;
		while (train_in >> str)
		{
			str = str + ",";
			data_processing(train_name, train_d, str, line);
			line++;
		}
		day = line - 1;

		for (int p = 0; p < test_times; p++)
		{
			main_function(train_name, train_d, day, p + 1, line, read_time);

			if (trend_global_max > trend_max)
			{
				best_count = 0;
				best_count++;
				best_test_time = p + 1;
				trend_max = trend_global_max;
				expected_return_max = expected_return_global_max;
				risk_max = risk_global_max;
				gen_max = gen_global_max;
				stock_choose_number_max = stock_choose_number_global_max;

				each_money_max.clear();
				for (int j = 0; j < day; j++)
				{
					for (int k = 0; k < stock_choose_number_max; k++)
					{
						each_money_max.push_back(each_money_global_max[k + j * stock_choose_number_max]);
					}
				}
				remain_money_max.clear();
				stock_buy_number_max.clear();
				index_max.clear();
				money_max.clear();
				result_max.clear();
				for (int k = 0; k < stock_number; k++)
				{
					result_max.push_back(result_global_max[k]);
				}
				for (int k = 0; k < stock_choose_number_max; k++)
				{
					remain_money_max.push_back(remain_money_global_max[k]);
					stock_buy_number_max.push_back(stock_buy_number_global_max[k]);
					index_max.push_back(index_global_max[k]);
				}
				for (int j = 0; j < day; j++)
				{
					money_max.push_back(money_global_max[j]);
				}
			}
			else if (trend_global_max == trend_max)
			{
				best_count++;
			}

			global_max_clear();
			probability.clear();
		}

		//output_each_slide_data(train_name, train_d, day, best_test_time, best_count, train_file[i].filename().generic_string());
		//output_all_slide_data(train_name, train_d, day, best_test_time, best_count, train_file[i].filename().generic_string());
		//output << "程式執行時間," << clock() / CLOCKS_PER_SEC << endl;

		line = 0; day = 0;
		local_clear();


		//測試期
		if (test_period_flag)
		{
			ifstream test_in(test_file[i]);
			while (test_in >> str)
			{
				//str = str + ",";
				data_processing(test_name, test_d, str, line);
				line++;
			}
			day = line - 1;

			if (trend_max == 0)
			{
				trend.push_back(0);
				expected_return.push_back(0);
				risk.push_back(0);

				for (int i = 0; i < day; i++)
				{
					money[0].push_back(test_initial_investment);
					each_money[0].push_back(test_initial_investment);
				}
				MDD_cal();
				//pf_cal();
			}
			else
			{
				if (stock_choose_number_max != 0)
				{
					test_cal(day, test_name, test_d, test_initial_investment);
				}
			}
			//output_test_each_slide_data(test_name, test_d, day, test_file[i].filename().generic_string(), test_initial_investment);
			//test_output(test_name, test_d, day, test_file[i].filename().generic_string(), test_initial_investment);
			test_initial_investment = money[0][money[0].size() - 1];
		}

		test_name.clear();
		test_d.clear();
		train_d.clear();
		train_name.clear();
		max_drop_down = 0.0;
		max_drop_down_ratio = 0.0;

		line = 0;

		best_test_time = 0;

		best_count = 0;

		probability.clear();
		trend.clear();
		stock_choose_number.clear();
		for (int g = 0; g < particle_number; g++)
		{
			index[g].clear();
			result[g].clear();
			money[g].clear();
			remain_money[g].clear();
			each_money[g].clear();
			stock_buy_number[g].clear();
		}
		result_global_max.clear();
		for (int i = 0; i < stock_number; i++)
		{
			result_global_max.push_back(0);
		}
		global_max_clear();

		train_name.clear();
		train_d.clear();
		stock_choose_number.clear();
		expected_return.clear();
		risk.clear();
		trend.clear();

		test_global_max_clear();
		result_max.clear();
	}
	if (negative_return == 0)
	{
		/*ofstream output("PF.csv", ios_base::app);
		output << fixed << setprecision(20);
		output << "Y2Q," << "inf" << endl;
		output.close();*/
	}
	else
	{
		/*ofstream output("PF.csv", ios_base::app);
		output << fixed << setprecision(20);
		output << "Y2Q," << positive_return / negative_return << endl;
		output.close();*/
	}
	/*ofstream output("MDD_AVE.csv", ios_base::app);
	output << fixed << setprecision(20);
	output << "Y2Q," << mdd_sum / test_file.size() << endl;*/

	//output << "程式總執行時間," << clock() / CLOCKS_PER_SEC << endl;
	//output.close();
}