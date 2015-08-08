/*
  wav file io

  2009�N7��
*/

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>

#include<cmath>
#include<vector>

#include<string>
#include<algorithm>

namespace wav{
  //WAV�t�@�C���̃w�b�_�B
  struct Header{
    char riff[4];
    unsigned int filesize;
    char wavefmt[8];
    unsigned int waveformat;
    unsigned short int pcm;
    unsigned short int n_channel;
    unsigned int sampling_rate;
    unsigned int bytes_per_second;
    unsigned short int block_per_sample;
    unsigned short int bits_per_sample;
    char data[4];
    unsigned int n_byte;
  };

  //1�`�����l���M���B
  class MonoChannel{
  public:
    MonoChannel(){}
    void init(int n){
      buffer_size = n;
      data = new double[n]; //�o�b�t�@�̑傫���͌��߂Ă��܂��B����ȏ���X�g�b�N���Ă������Ƃ͂ł��Ȃ��B
      current_point = 0;
    }
    ~MonoChannel(){
      delete []data;
    }
    void insert(double x){
      data[current_point] = x;
      current_point = (current_point < buffer_size - 1 ? current_point + 1 : 0);
    };
    double& operator[](int i){
      return data[i + current_point < buffer_size ? i + current_point : i + current_point - buffer_size];
    }
  private:
    double *data;
    int current_point; //�����O�o�b�t�@���Ǘ����邽�߂Ɏg���B
    int buffer_size;
  };
  //���`�����l���M�� �� 1�`�����l���M������������ێ�����B
  class Signal{
  public:
    Signal(){}
    void init(int n_channel, int buf_size){
      buffer_size = buf_size;
      data = new MonoChannel[n_channel];
      for(int i = 0; i < n_channel; i++){
        data[i].init(buf_size);
      }
    }
    ~Signal(){
      delete []data;
    }
    int length(){return buffer_size;}//����Ȋ֐��K�v���낤���H
    MonoChannel& operator[](int n){return data[n];}
  private:
    MonoChannel *data;
    int buffer_size;
  };
  //�������C�ʎq���̊֐��B
  //unsigned char��signed short����C�����ɕϊ�����֐��B
  template<typename T> double realization(T){return 0.0;}; //���̌`�ł͎g��Ȃ����CC++�̕��@��K�v�Ȃ̂ŏ����Ă��邾���B
  template<> double realization(unsigned char x){return static_cast<double>(x + 128) / 256.0;};
  template<> double realization(signed short x) {return static_cast<double>(x)/32768.0;};
  //double����C���ꂼ���unsigned char��Csigned short�ɖ߂��֐��B
  template<typename T> T quantize(double){;}; //���̌`�ł͎g��Ȃ����CC++�̕��@��K�v�Ȃ̂ŏ����Ă���B
  template<> unsigned char quantize<unsigned char>(double x){return static_cast<unsigned char>(x * 256 - 128);};
  template<> signed short  quantize<signed short> (double x){return static_cast<signed short> (x * 32768);};
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavistream�N���X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class wavistream{
public:
  wavistream(const char*, int);
  ~wavistream();
  void read(int n){if(header.bits_per_sample == 8)  __read<unsigned char>(n); else  __read<signed short>(n);};
  void copy(double* x, int ch, int n){for(int i = 0; i < n; i++)x[i] = signal[ch][i];};//�Ƃ肠�����������Ă����B
  bool eof(){return feof(fp);}
  wav::Header header;
private:
  wav::Signal signal;
  std::string filename;
  FILE *fp;
  int current_seek; //���݃t�@�C���̒��̂ǂ̒n�_��ǂݍ���ł���̂����L���B
  template<typename T> void __read(int n);
};
wavistream::wavistream(const char *fn, int buffer_size){
  //�t�@�C���|�C���^���擾
  filename = fn;
  fp = fopen(filename.c_str(), "r");
  if( !fp ){
    std::cerr << "cannot open " << filename << "!" <<std::endl;
    exit(1);
  }
  //�w�b�_�̓ǂݍ��݁B
  fread(&header, sizeof(header), 1, fp);
  current_seek = 0;//sizeof(header); ��҂͊ԈႢ�B�������Ԉ���Ă���o�[�W���������݂���̂ŗv����
  signal.init(header.n_channel, buffer_size);
}
wavistream::~wavistream(){
  fclose(fp);
}
template<typename T> void // unsigned char or signed short
wavistream::__read(int n){
  T tmp;
  double tmp_double;
  //���o��
  fseek(fp, sizeof(wav::Header) + current_seek, SEEK_SET);
  //�f�[�^�̓ǂݍ��݁Bwav�t�@�C���ł̓`�����l�����ɕ���ł���B
  for(int i = 0; i < n ; i++){
    for(int ch = 0; ch < header.n_channel; ch++){
      if(!eof()){
        //�I�[�o�[�������Ȃ��悤�ɁB�����x���悤�Ȃ�l�X�g�̏��Ԃ��������B
        fread(&tmp, sizeof(T), 1, fp);
        tmp_double = wav::realization(tmp);
        signal[ch].insert( tmp_double );
      }else{
        signal[ch].insert( 0.0 );
      }
    }
  }
  current_seek += n * header.n_channel * sizeof(T);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavistream�N���X�����B
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavostream�N���X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�܂��݌v���ǂ��Ȃ��Bwav���p�����郂�f���ł����̂��Hread�Ƃ̑Ώ̐����K�v�ł͂Ȃ����H���p���͂�߂��B
class wavostream{
public:
  wavostream(const char*, int, int);
  ~wavostream();
  void set(double*, int, int);
  void write(int n){if(header.bits_per_sample == 8)  __write<unsigned char>(n); else  __write<signed short>(n);};;
  void write_header();
  wav::Header header;
  
private:
  wav::Signal signal;
  std::string filename;
  FILE *fp;
  template<typename T> void __write(int);
  int signal_length;
  int buffer_size;
};

wavostream::wavostream(const char *fn, int ch, int buffer_size){
  //�t�@�C���|�C���^���擾
  filename = fn;
  fp = fopen(filename.c_str(), "w");
  if( !fp ){
    std::cerr << "cannot open " << filename << "!" <<std::endl;
    exit(1);
  }
  signal_length = 0;
  char tmp[sizeof(wav::Header)];
  //�w�b�_�����͐��0�Ŗ��߂Ă����B
  fwrite(tmp, 1, sizeof(wav::Header), fp);
  signal.init(ch, buffer_size);
  header.n_channel = ch;
  this->buffer_size = buffer_size;
}
wavostream::~wavostream(){
  write_header();
  fclose(fp);
}

void wavostream::set(double *x, int ch, int n){
  //�܂��̓T�`�����[�V�������Ȃ����B�ő�l���擾����������ƁC�s���R�B
//  double max = *std::max_element(&(x[0]), &(x[n]));
  for(int t = 0; t < n; t++){
    if(x[t] > 1.0){
      signal[ch].insert(1.0);
    }else if(x[t] < -1.0){
      signal[ch].insert(-1.0);
    }else{
      signal[ch].insert(x[t]);
    }
  }
}

//�f�[�^�����ۂɏ������݁B
//�����Ƃ��̓f�[�^�𓪂���ł͂Ȃ������肩�珑���B
template<typename T> void // unsigned char or signed short
wavostream::__write(int n){
  for(int j = buffer_size - n; j < buffer_size; j++){
    for(int ch = 0; ch < header.n_channel; ch++){
      //double�̌n����Cunsigned char, signed short�ɗʎq������B
      T tmp = wav::quantize<T>(signal[ch][j]);
      fwrite(&tmp, sizeof(T), 1, fp);
    }
  }
//  current_seek += n * header.n_channel * sizeof(T);
  //�w�b�_�ɕύX��������B
  signal_length += n;
}

void wavostream::write_header(void){
  if(header.sampling_rate == 0 || header.n_channel == 0 || header.bits_per_sample == 0){
    std::cerr << "sampling_rate, n_channel & bit_rate are not set!" << std::endl;
    exit(1);
  }

  //�w�b�_�̊e�v�f�ɕЂ��ς��������B
  header.riff[0] = 'R';
  header.riff[1] = 'I';
  header.riff[2] = 'F';
  header.riff[3] = 'F';
  header.filesize = (header.bits_per_sample / 8) * signal_length * header.n_channel + 36;
  header.wavefmt[0] = 'W';
  header.wavefmt[1] = 'A';
  header.wavefmt[2] = 'V';
  header.wavefmt[3] = 'E';
  header.wavefmt[4] = 'f';
  header.wavefmt[5] = 'm';
  header.wavefmt[6] = 't';
  header.wavefmt[7] = ' ';
  header.waveformat = header.bits_per_sample;
  header.pcm = 1;
  header.n_channel = header.n_channel;
  header.sampling_rate = header.sampling_rate;
  header.bytes_per_second = (header.bits_per_sample / 8) * header.n_channel * header.sampling_rate;
  header.block_per_sample = (header.bits_per_sample / 8) * header.n_channel;
  header.bits_per_sample = header.bits_per_sample;
  header.data[0] = 'd';
  header.data[1] = 'a';
  header.data[2] = 't';
  header.data[3] = 'a';
  header.n_byte = (header.bits_per_sample / 8) * signal_length * header.n_channel; //���̕ӂ̌v�Z�͌������K�v������B
  fseek(fp, 0, SEEK_SET);
  fwrite(&header, sizeof(header), 1, fp);

}

