#include <float.h>
#include <iostream>
#include <random>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
vector<vector<double>> m1_e;//erosion
vector<vector<double>> m2_e;
vector<vector<double>> m1_d;//dilation
vector<vector<double>> m2_d;
vector<vector<bool>> temp;
typedef struct messageToThread {

    size_t left;
    size_t right;
    bool flag_x;

} messageToThread;

messageToThread newMessage(size_t left, size_t right, bool flag)
{
    messageToThread msg;
    msg.left = left;
    msg.right = right;
    msg.flag_x = flag;

    return msg;
}
void erosion_matrix(size_t left, size_t right, bool flag_x)
{
    size_t n_tempalate = temp.size();
    size_t lines = (flag_x == 0) ? right : m1_e.size();
    size_t rows = (flag_x == 1) ? right : m1_e[0].size();
    size_t x_beg = (flag_x == 0) ? left : 0;
    size_t y_beg = (flag_x == 1) ? left : 0;
    for (size_t x = x_beg; x < lines; ++x) {
        for (size_t y = y_beg; y < rows; ++y) {

            double el_cur = DBL_MAX;

            for (size_t i = 0; i < n_tempalate; ++i) {

                for (size_t j = 0; j < n_tempalate; ++j) {

                    if (temp[i][j]) {
                        if (x + i >= n_tempalate / 2 && x + i < m1_e.size() + n_tempalate / 2 && y + j >= n_tempalate / 2 && y + j < m1_e[0].size() + n_tempalate / 2) {
                            el_cur = min(el_cur, m1_e[x + i - n_tempalate / 2][y + j - n_tempalate / 2]);
                        }
                    }
                }
            }
            m2_e[x][y] = el_cur;
        }
    }
}
void dilation_matrix(size_t left, size_t right, bool flag_x)
{
    size_t n_tempalate = temp.size();
    size_t lines = (flag_x == 0) ? right : m1_d.size();
    size_t rows = (flag_x == 1) ? right : m1_d[0].size();
    size_t x_beg = (flag_x == 0) ? left : 0;
    size_t y_beg = (flag_x == 1) ? left : 0;
    for (size_t x = x_beg; x < lines; ++x) {
        for (size_t y = y_beg; y < rows; ++y) {
            double el_cur = DBL_MIN;

            for (size_t i = 0; i < n_tempalate; ++i) {

                for (size_t j = 0; j < n_tempalate; ++j) {
                    if (temp[i][j]) {
                        if (x + i >= n_tempalate / 2 && x + i < m1_d.size() + n_tempalate / 2 && y + j >= n_tempalate / 2 && y + j < m1_d[0].size() + n_tempalate / 2) {
                            el_cur = max(el_cur, m1_d[x + i - n_tempalate / 2][y + j - n_tempalate / 2]);
                        }
                    }
                }
            }
            m2_d[x][y] = el_cur;
        }
    }
}
void *threadFunction(void *arg)
{
    messageToThread *msg = (messageToThread *) arg;
    erosion_matrix(msg->left, msg->right, msg->flag_x);
    dilation_matrix(msg->left, msg->right, msg->flag_x);

    pthread_exit(NULL);
}
int main(int argc, char **argv)
{


    int threadsN = 16;
    if (argc == 2) {
        threadsN = atoi(argv[1]);
    }
    cout << "Will calculate on" << threadsN << " threads\n"
         << endl;

    int64_t n, m, l, a, k;

    cin >> k;
    cin >> n;
    cin >> m >> l;
    bool flag = 0;
    m1_e.resize(m, vector<double>(l));
    m2_e.resize(m, vector<double>(l));
    m1_d.resize(m, vector<double>(l));
    m2_d.resize(m, vector<double>(l));
    temp.resize(n, vector<bool>(n, 1));
    for (int64_t x = 0; x < m; ++x) {
        for (int64_t y = 0; y < l; ++y) {
            //cin>>m1_e[x][y];
            m1_e[x][y] = rand() * 0.1;
            m1_d[x][y] = m1_e[x][y];
        }
        //cout<<endl;
    }
    /*for(int64_t x=0;x<m;++x){
        for(int64_t y=0;y<l;++y){

                cin>>a;
                temp[x][y]=a;

        }

    }*/
    clock_t begin = clock();
    for (int64_t i = 0; i < k; ++i) {

        pthread_t *threads = (pthread_t *) malloc(threadsN * sizeof(pthread_t));
        messageToThread *msg = (messageToThread *) malloc(threadsN * sizeof(messageToThread));

        int64_t left = 0;
        int64_t right = 0;
        for (int i = 0; i < threadsN; ++i) {
            if (m >= l) {
                flag = 0;
                right = min(m, left + (m + threadsN - 1) / threadsN);
            }
            else {
                flag = 1;
                right = min(l, left + (l + threadsN - 1) / threadsN);
            }

            msg[i] = newMessage(left, right, flag);
            left = right;

            if (pthread_create(&threads[i], NULL, threadFunction, (void *) &msg[i]) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < threadsN; ++i) {
            pthread_join(threads[i], NULL);
        }
        free(threads);
        free(msg);
        swap(m1_e, m2_e);
        swap(m1_d, m2_d);
    }
    clock_t end = clock();
    printf("elapsed %3.6f ms\n", ((double) (end - begin) / CLOCKS_PER_SEC) * 1000);
    /* for(int64_t x=0;x<m;++x){
        for(int64_t y=0;y<l;++y){
            cout<<m1_e[x][y]<<"  ";
        }
        cout<<endl;
    }
      for(int64_t x=0;x<m;++x){
        for(int64_t y=0;y<l;++y){
            cout<<m1_d[x][y]<<"  ";
        }
        cout<<endl;
    }*/

    return 0;
}
