#include "BIDSSensorLayer.hpp"

namespace PV{

BIDSSensorLayer::BIDSSensorLayer(){
   initialize_base();
}

BIDSSensorLayer::BIDSSensorLayer(const char * name, HyPerCol * hc, int numChannels){
   initialize_base();
   initialize(name, hc, numChannels);
}

BIDSSensorLayer::BIDSSensorLayer(const char * name, HyPerCol * hc){
   initialize_base();
   initialize(name, hc);
}

int BIDSSensorLayer::initialize_base(){
   data = NULL;
//   coords = NULL;
   buf_size = 0;
   neutral_val = 0;
   blayer = NULL;
   nx = 0;
   ny = 0;
   nf = 0;
   buf_index = 0;
   return PV_SUCCESS;
}

int BIDSSensorLayer::initialize(const char * name, HyPerCol * hc, int numChannels){
   HyPerLayer::initialize(name, hc, numChannels);

   //Grab Parameters
   float nxScale = (float)(parent->parameters()->value(name, "nxScale"));
   float nyScale = (float)(parent->parameters()->value(name, "nyScale"));
   int HyPerColx = (int)(parent->parameters()->value("column", "nx"));
   int HyPerColy = (int)(parent->parameters()->value("column", "ny"));

   freq = (float)(parent->parameters()->value(name, "frequency"));
   ts = (float)1/((float)(parent->parameters()->value(name, "ts_per_period")) * freq);
   buf_size = (float)(parent->parameters()->value(name, "buffer_size"));
   neutral_val = (float)(parent->parameters()->value(name, "neutral_val"));
   weight = (float)(parent->parameters()->value(name, "weight"));

   //Grab coords
   const char * jitterSourceName = parent->parameters()->stringValue(name, "jitterSource");
   blayer = dynamic_cast<BIDSMovieCloneMap*> (parent->getLayerFromName(jitterSourceName));
   assert(blayer != NULL);
   coords = blayer->getCoords();
   numNodes = blayer->getNumNodes();

   //Set nx and ny
   nx = nxScale * HyPerColx;
   ny = nyScale * HyPerColy;
   nf = blayer->getLayerLoc()->nf;

   //Create data structure for data
   //data[Node_id][Buffer_id]
   float* datatemp = (float*)malloc(sizeof(float)*numNodes*buf_size);
   data = (float**)malloc(sizeof(float*)*numNodes);

   //Initialize data structure
   for(int i = 0; i < numNodes; i++){
      data[i] = &datatemp[i*buf_size];
      for(int j = 0; j < buf_size; j++){
         data[i][j] = 0;
      }
   }

   //Get perfect match for match filter to normalize output
   return PV_SUCCESS;
}

int BIDSSensorLayer::updateState(float timef, float dt){
   pvdata_t * output = getCLayer()->V;
   pvdata_t * input = blayer->getCLayer()->activity->data;
   int index;
   //Iterate through post layer
   for (int i = 0; i < nx * ny; i++){
      assert(nf == 1);
      //Iterate through features
//      std::cout << "Node (" << coords[i].xCoord << ", " << coords[i].yCoord << ")\n";
      for (int k = 0; k < nf; k++){
         int x = i % nx;
         int y = (int) floor(i/nx);
         index = kIndex(x, y, k, nx, ny, nf);
         data[i][buf_index] = input[index] - (neutral_val / 256);
//         std::cout << "\tBuf_index: " << buf_index << ": " << data[i][buf_index] << "\n";
         //Next buf index, or reset if at end
         float out = matchFilter(i, (int)(timef * dt));
         output[index] = out * weight;
      }
   }
   if(buf_index < buf_size - 1){
      buf_index++;
   }
   else{
      buf_index = 0;
   }
   HyPerLayer::setActivity();
   return PV_SUCCESS;
}

float BIDSSensorLayer::matchFilter(int node_index, int frame_index){
   float sin_sum = 0;
   float cos_sum = 0;
   float double_sin_sum = 0;
   float half_sin_sum = 0;
   float double_cos_sum = 0;
   float half_cos_sum = 0;

   //Filter for the buffer:
   for(int j = 0; j < buf_size; j++){
      //Buf_index + 1 is the oldest data point
      float d_point = data[node_index][(buf_index + 1 + j) % buf_size];
      //Freq that is being detected
      float sin_point = sin(freq * ts * 2 * PI * j);
      float cos_point = cos(freq * ts * 2 * PI * j);
      //Double freq for baseline
      float double_sin_point = sin(2 * freq * ts * 2 *  PI * j);
      float double_cos_point = cos(2 * freq * ts * 2 * PI * j);
      //Half freq for baseline
      float half_sin_point = sin(.5 * freq * ts * 2 * PI * j);
      float half_cos_point = cos(.5 * freq * ts * 2 * PI * j);

      sin_sum += d_point * sin_point;
      cos_sum += d_point * cos_point;
      double_sin_sum += d_point * double_sin_point;
      double_cos_sum += d_point * double_cos_point;
      half_sin_sum += d_point * half_sin_point;
      half_cos_sum += d_point * half_cos_point;
   }

   float C = sqrt(pow(sin_sum, (float)2) + pow(cos_sum, (float)2));
   float double_C = sqrt(pow(double_sin_sum, (float)2) + pow(double_cos_sum, (float)2));
   float half_C = sqrt(pow(half_sin_sum, (float)2) + pow(half_cos_sum, (float)2));
   float normalize_val = ((float)2 * double_C + .5 * half_C) / (float)2;

   float out = (log(C/normalize_val)/log((float)2));
   return out;
}

void BIDSSensorLayer::writeCSV(std::string fname, int node_index){
   std::string dir = "/Users/slundquist/Documents/workspace/BIDS/ouput/";
   fname = dir + fname;
   std::ofstream file;
   //Open file, overwriting if it exists already
   file.open (fname.c_str(), std::ios::out | std::ios::trunc);
   for(int j = 0; j < buf_size; j++){
      float d_point = data[node_index][(buf_index + 1 + j) % buf_size];
      file << d_point << "\n";
   }
   file.close();
}

BIDSSensorLayer::~BIDSSensorLayer(){
   //All data allocated contiguously
   free(data[0]);
   free(data);
}


}
