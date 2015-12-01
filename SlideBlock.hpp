
#ifndef SLIDE_BLOCK_HEADER
#define SLIDE_BLOCK_HEADER

#include<cstdio>
#include<vector>

template<typename T>
class SlideBlock{
public:
  SlideBlock(int t, int k);
  ~SlideBlock();
  
  void apply(T(*)(T)); // �S���̗v�f�Ɉ����Ƃ��ė^�����֐���K�p����B���������1��Ԃ��C���s�����0��Ԃ��B
  T at(int t, int k); // �w�肳�ꂽ�v�f��Ԃ��B�Y�����`�F�b�N������o�[�W�����B
  T* operator[](int t); // �w�肳�ꂽ�v�f��Ԃ��悤�ȁC�ȒP�ȃC���^�[�t�F�[�X�B�Y�����`�F�b�N�����Ȃ��B
  void push(T *in); // �f�[�^��}������B���pop���Ȃ��ƁC�㏑������ăf�[�^�������܂��B
  void push(std::vector<T> in); // �f�[�^��}������B���pop���Ȃ��ƁC�㏑������ăf�[�^�������܂��B
  void pop(T *out); // �f�[�^����肾���B
  std::vector<T> pop(); // �f�[�^����肾���B
  
protected:
  int n_time; // �t���[���̐�
  int n_freq; // ���g��bin�̐�
  T* data;

private:
  int current_point;
  T** data_alias;
};

template<typename T>
SlideBlock<T>::SlideBlock(int n, int k)
     :n_time(n),
      n_freq(k),
      current_point(0)
{
  data = new T[n * k];
  data_alias = new T*[n];
  for(int i = 0; i != n; i++){
    data_alias[i] = &(data[i * k]);
  }
  //�S���[���l
  for(int i = 0; i < n * k; i++){
    data[i] = 0;
  }
}

template<typename T>
SlideBlock<T>::~SlideBlock(){
  delete[] data;
  delete[] data_alias;
}

template<typename T>
T SlideBlock<T>::at(int t, int k){
  t = (t - current_point);
  if(t < 0){
    t = t + n_time;
  }
  if(0 <= t < n_time && 0 <= k < n_freq){
    return data_alias[t][k];
  }else{
    printf("Error at SlideBlock.hpp\nToo large index: time index: %d and frequency index %d", t, k);
    exit(1);
  }
}

template<typename T>
T* SlideBlock<T>::operator[](int t){
  t += current_point;
  if(t >= n_time){
    t -= n_time;
  }
  return data_alias[t];
}

template<typename T>
void SlideBlock<T>::apply(T(*func)(T)){
  for(int i = 0; i < n_time * n_freq; i++)
    data[i] = func(data[i]);
}

template<typename T>
void SlideBlock<T>::push(T* in){
  for(int i = 0; i != n_freq; i++)
    data_alias[current_point][i] = in[i];
  current_point++;
  if(current_point == n_time)
    current_point = 0;
}
template<typename T>
void SlideBlock<T>::push(std::vector<T> in){
  if(in.size() != n_freq){
    printf("Error at SlideBlock.hpp, push(std::vector<T>): size of std::vector is invalid.\n");
    exit(1);
  }
  for(int i = 0; i != n_freq; i++)
    data_alias[current_point][i] = in[i];
  current_point++;
  if(current_point == n_time)
    current_point = 0;
}

template<typename T>
void SlideBlock<T>::pop(T* out){
  for(int i = 0; i != n_freq; i++)
    out[i] = data_alias[current_point][i];
}

#endif



