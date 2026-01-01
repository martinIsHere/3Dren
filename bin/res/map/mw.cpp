#include <fstream>

int main(){

	std::fstream mapFile;
	mapFile.open("map.bin", std::ios::out|std::ios::binary);
	unsigned int width = 50;
	unsigned int height = 50;
	uint16_t front_ID = 0x0001;
	uint16_t back_ID = 0x0005;
	unsigned char solidState = 0x1;
	unsigned char buffer;
	buffer = char((width>>8) & 0xFF);
	mapFile << buffer;
	buffer = width & 0xFF;
	mapFile << buffer;
	buffer = (height>>8) & 0xFF;
	mapFile << buffer;
	buffer = height & 0xFF;
	mapFile << buffer;
	
	for(int x = 0; x < 50; x++){
		for(int y = 0; y < 50; y++){
			buffer = (front_ID>>8) & 0xff;
			mapFile << buffer;
			buffer = front_ID & 0xff;
			mapFile << buffer;
			buffer = (back_ID>>8) & 0xff;
			mapFile << buffer;
			buffer = back_ID & 0xff;
			mapFile << buffer;
			buffer = solidState & 0x1;
			buffer += 0b10;
			buffer += 0b1000;
			mapFile << buffer;
		}
	}
	mapFile.close();
	return 0;
	// it works fine
}