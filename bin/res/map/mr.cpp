#include <fstream>

int main(){

	std::fstream mapFile;
	mapFile.open("map.txt");
	mapFile << "50 ";
	mapFile << "50 ";
	unsigned int width = 50;
	unsigned int height = 50;
	uint16_t front_ID = 1;
	uint16_t back_ID = 1;
	unsigned char solidState = 1;
	unsigned char buffer;

	

	for(int x = 0; x < 50; x++){
		for(int y = 0; y < 50; y++){
			buffer = (height>>8) & 0xFF;
			mapFile << buffer;
			buffer = height & 0xFF;
			mapFile << buffer;
			buffer = (front_ID>>8) & 0xff;
			mapFile << buffer;
			buffer = front_ID & 0xff;
			mapFile << buffer;
			buffer = (back_ID>>8) & 0xff;
			mapFile << buffer;
			buffer = back_ID & 0xff;
			mapFile << buffer;
			buffer = solidState & 0xff;
			mapFile << buffer;
		}
	}
	mapFile.close();
	return 0;
}