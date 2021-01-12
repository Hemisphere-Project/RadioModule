/*
TEA5767 FM Radio Library
Using I2C for Communication
big12boy - 2017
*/

#include <Arduino.h>
#include <TEA5767.h>

TEA5767::TEA5767(){
	_addr = 0x60;
	_lvl = 2; _sel = 0; _staCnt = 0;
	_freqH = 0x00; _freqL = 0x00;
	_muted = false; _search = false; _up = true; _stby = false; _snc = true;
}

//Send Data to the Module
void TEA5767::send(){  	
	_wire->beginTransmission(_addr); 
	_wire->write((_muted << 7) | (_search << 6) | _freqH);
	_wire->write(_freqL);
	_wire->write((_up << 7) | (_lvl & 0x3 << 5) | 0x10);
	_wire->write(0x10 | (_stby << 6));
	_wire->write(0x00);
	_wire->endTransmission();
}
//Get Data from the Module
void TEA5767::get(){  	
	byte wIn;
	_wire->requestFrom(_addr, 5);
	while(_wire->available() < 5);
	
	wIn = _wire->read();
	bool rf = wIn & 0x80;
	bool blf = wIn & 0x40;
	if(!rf)_rdy = 0;
	else if(rf && !blf)_rdy = 1;
	else _rdy = 2;
	_freqH = wIn & 0x3F;
	
	wIn = _wire->read();
	_freqL = wIn;
	
	wIn = _wire->read();
	_stereo = wIn & 0x80;
	
	wIn = _wire->read();
	_lvl = wIn >> 4;
	
	wIn = _wire->read();	
}

//Initialize and get available Stations
short TEA5767::init(TwoWire* theWire, short minlvl){
	_wire = theWire;
	// findStations(minlvl);	
	return _staCnt;
}

//Initialize and get available Stations
short TEA5767::init(short minlvl){
    Wire.begin();	
	_wire = &Wire;
	// findStations(minlvl);	
	return _staCnt;
}

//Set Functions
bool TEA5767::setFrequency(float frequency){
	if(frequency < 87.50 || frequency > 108.00) return false;
	
  	unsigned int freqC = (frequency * 1000000 + 225000) / 8192; 
	_freqH = freqC >> 8;
	_freqL = freqC & 0XFF;	
	send();
	return true;
}
void TEA5767::setMuted(bool muted){
	_muted = muted;	
	send();
}
bool TEA5767::setSearch(bool up, int level){
	_up = up;
	if(level < 1 || level > 3) return false;
	_lvl = level;
	_search = true;
	send();
	_search = false;
	//send();
	return true;
}
void TEA5767::setStandby(bool stby){
	_stby = stby;	
	send();
}
void TEA5767::setStereoNC(bool snc){
	_snc = snc;	
	send();
}

//Get Functions
float TEA5767::getFrequency(){
	get();	
	double freqI = (_freqH << 8) | _freqL;
	return (freqI * 8192 - 225000) / 1000000.00; 
}
int TEA5767::getReady(){
	get();	
	return _rdy;
}
bool TEA5767::isStereo(){
	get();	
	return _stereo;
}
short TEA5767::getSignalLevel(){
	get();	
	return _lvl;
}
bool TEA5767::isMuted(){	
	return _muted;
}

//Stations
int TEA5767::findStations(short minlvl){
	short llvl = 0;
	float lsta = 0.00;
	_staCnt = 0;
	
	for(float curfreq = 87.50; curfreq <= 108.00; curfreq+=0.1){
		setFrequency(curfreq);		
		delay(80);
		get();
		
		if(_lvl >= minlvl && _stereo){
			_stations[_staCnt] = curfreq;
			if(lsta >= (curfreq - 0.30)){
				if(llvl <= _lvl) _stations[_staCnt - 1] = curfreq;
			}
			else _staCnt++;	
			
			lsta = curfreq;
			llvl = _lvl;
		}	  
	}
	
	if(_staCnt > 0)setFrequency(_stations[0]);
	return _staCnt;
}
float TEA5767::nextStation(){
	float sta;
	if(_staCnt == 0) return 0.00; //No Stations Stored
	else if(_staCnt == 1) sta = _stations[0];
	else{
		_sel++;
		if(_sel >= _staCnt) _sel = 0;
		sta = _stations[_sel];
	}
	
	setFrequency(sta);
	return sta;		
}
short TEA5767::getStations(){
	return _staCnt;		
}