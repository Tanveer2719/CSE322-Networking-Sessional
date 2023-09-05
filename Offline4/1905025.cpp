#include<bits/stdc++.h>
using namespace std;

/**
 * find the xor of two strings
 * Also ignore any forward 0's resulting after the xor
*/
string xorString(string s1, string s2){
    string ret = "";
    bool oneFound = false;
    int n = s1.length();
    for(int i = 0; i<n; i++){
        if(s1[i] == s2[i]){
            if(oneFound)
                ret += "0";
        }else{
            if(! oneFound) oneFound = true;
            ret +="1"; 
        }
    }

    // cout<<"given String: "<<s1 << " and "<<s2<< " returned String: "<<ret << endl;
    return ret;
}

/**
 * find the xor of two char and return a char
*/
char xorChar(char c, char d){
    // cout<<"c = "<<c << " d = "<<d<<endl;
    if(c == d)return '0';
    else return '1';
}

/**
 * Returns the remainder after performing mod2Div
 * \param vajjo what to be divided
 * \param vajok who will divide
 * 
*/
string division(string vajjo, string vajok){
    // cout<<"vajjo: "<<vajjo<<endl;
    int n = vajok.length();
    int vajjoLength = vajjo.length();
    int i, j;
    
    //skip the initial 0's 
    for(i = 0; i<vajjoLength; i++){
        if(vajjo[i] == '1') break;
    }
    vajjo = vajjo.substr(i,vajjoLength);
    // cout<<"vajjo: "<<vajjo<<endl;
    vajjoLength = vajjo.length();

    // if the vajjo is less than vajok then the vajjo is the remainder
    if(vajjoLength < n){
        return vajjo;
    }

    string tmp = vajjo.substr(0, n); // pick the substring equal to the vajok
    j = tmp.length();
    bool flag = false;
    while(true){
        tmp = xorString(tmp, vajok); 
        int tmpSize = tmp.length();

        // xorString supposed to return a string with no forward 0
        bool oneFound = tmpSize > 0 ? true: false;

        for(; tmp.length()<n; j++){
            if(j == vajjoLength){
                flag = true;
                break;
            }
            if(vajjo[j] == '0' && !oneFound) continue; // skip the forward  0's
            else if(vajjo[j] == '1' && !oneFound) oneFound = true;  // if a 1 found change the flag
            tmp+=vajjo[j];
        }
        if(flag) break;

    }
    if(tmp.length()) return tmp;
    else return "0";
    
}

void padTheDataString(string &dataString, int bytesPerRow){
    // cout<<dataString.size()<<" "<<bytesPerRow<<endl;
    int r, x = (dataString.size() % bytesPerRow);
    if(x) r = bytesPerRow - x;
    else r =  0;

    for(int i = 0; i<r; i++){
        dataString += "~";
    }
}

void input(string &data,int &bytesPerRow, float &toggleProbability, string &polynomial ){
    cout<<"enter data string: ";
    getline(cin, data);
    cout<<"enter number of data bytes in a row <m>: ";
    cin>>bytesPerRow;
    cout<<"enter probability <p>: ";
    cin>>toggleProbability;
    while(toggleProbability >1 || toggleProbability < 0){
        cout<<"wrong input !! try again: ";
        cin>>toggleProbability;
    }
    cout<<"enter generator polynomial: ";
    // the higher order bit of the generator should be 1 
    cin>>polynomial;
    while(polynomial[0] == '0'){
        cout<<"the higher order bit of the generator should be 1\n";
        cin>>polynomial;
    }

    cout<<endl;
}

/**
 * \param data the data string
 * \param m no of characters per row
 * \param s starting index of the data string
*/
string convertToBits(string &data, int m, int s){
    string str = "";
    for(int i = s; i<s+m; i++){
        bitset<8> bits(data[i]);
        str += bits.to_string();
    }
    return str;
}

/**
 * add checkBits using hamming code
*/
void addCheckBits(string &s){
    int m = s.length(), r = 0, rhs = 1;
    
    // calculate r using the relation (m+r+1) <= 2^r
    while((m+r+1) > rhs){
        r++;
        rhs *= 2;
    }
    char charArray[m+r];
    charArray[0]='a', charArray[1]='a';
    int i = 2,j = 2, k = 0;
    while(k < m){
        // at the positions power of 2 insert 'a'
        if(j == ((i << 1)-1)){ 
            i = (i << 1);
            charArray[j++] = 'a';
            continue;
        }
        // insert the data bit in other positions
        charArray[j] = s[k];    
        j++;k++;
    }    

    int rBit = 0;   //the check bit
    
    for(int i = 1; i<= m+r; i <<= 1){
        int count = 0; // count the no. of 1's
        for(j = i+1; j<= m+r; j++){
            if(j & (1<<rBit)){
                if(charArray[j-1] == '1') count++;
            }
        }
        if(count & 1){      // count is odd
            charArray[i-1] = '1';
        }else{
            charArray[i-1] = '0';
        }
        rBit ++;
    }
    s = "";
    for(auto x: charArray){
        s+=x;
    }
}

void oneBitErrorCorrection(string &s){ 
    int m = s.length();

    string ans = ""; 

    int rBit = 0;   //the check bit
    for(int i = 1; i<= m; i <<= 1){ // 1,2,4,8,16
        char c = s[i-1];
        for(int j = i+1; j<= m; j++){
            if(j & (1<<rBit)){
                // cout<<"j = "<<j<<endl;
                c = xorChar(s[j-1], c);
            }
        }
        ans.push_back(c); 
        rBit ++;
    }

    reverse(ans.begin(), ans.end());
    int pos = (int)bitset<64>(ans).to_ulong();
    if (pos > 0) {
        // cout << "The wrong bit is at position: " << pos << endl;
        s[pos - 1] = (s[pos - 1] == '0') ? '1' : '0'; // Correct the bit
    }

}


void removeCheckBits(string &s){
    int posOfCheckBit = 1;
    int len = s.length();
    for(int i = 1; i<= len; i++){
        if(posOfCheckBit == i){
            s[i-1] = 'a';
            posOfCheckBit = posOfCheckBit << 1;
        }
    }
    // remove(), moves all occurence of a to the last of the string
    // cout<<s<<endl;
    s.erase(remove(s.begin(), s.end(), 'a'), s.end());
    // cout<<s<<endl;
}
/**
 * adds the crc Checksum at the end of the dataBits
*/
string encode(string dataBits, string generator){
    int n = generator.length();
    
    // add the 0's at the end of the string 
    string tmpDataBits = (dataBits + string(n - 1, '0'));
    dataBits = tmpDataBits;
    string remainder = division(tmpDataBits, generator);

    // cout<<"remainder: "<<remainder<<endl;

    // pad the remainder with the dataBits
    n = dataBits.length()-1;
    for(int i = remainder.length()-1; i>=0; i--){
        dataBits[n--] = remainder[i];
    }
    return dataBits;
}

/**
 * toggle the bits with probability p
*/
vector<int> toggleBits(string bitString, float p, mt19937 &eng){
    vector<int>indexes;
    for(int i = 0; i<bitString.length(); i++){
        uniform_real_distribution<double> dist(0.0, 1.0);
        double randomValue = dist(eng);
        if(randomValue < p)
            indexes.push_back(i);
    }
    return indexes; 
}


int main(){
    // Seed the random number generator using random_device and mt19937
    random_device r;
    seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
    mt19937 eng(seed);

    string dataString, generatorPolynomial;
    int bytesPerRow;
    float toggleProbability = -1;
    input(dataString, bytesPerRow, toggleProbability, generatorPolynomial);

    // make sure data string is multiple of m
    padTheDataString(dataString, bytesPerRow);
    cout<<"\ndata string after padding: "<<dataString<<endl;
    
    // convert the dataString to m bytes per row
    vector<string>bitArray;
    int rows = dataString.size()/bytesPerRow;
    bitArray.resize(rows);
    for(int i = 0, j = 0; i<dataString.size()/bytesPerRow; i++, j+= bytesPerRow){
        bitArray[i] = convertToBits(dataString, bytesPerRow, j);
    }
    cout<<"\ndata block <ascii code of m characters per row>:\n";
    for(auto x: bitArray){
        cout<<x<<endl;
    }

    cout<<"\ndata block after adding check bits:\n";
    for(int x = 0; x < bitArray.size(); x++){
        addCheckBits(bitArray[x]);
        int coloredIndx = 1;
        for(int i = 0; i<bitArray[x].size(); i++){
            if(i == (coloredIndx-1)){
                cout<<"\033[32m"<<bitArray[x][i]<<"\033[0m";
                coloredIndx *= 2;
            }
            else{
                cout<<bitArray[x][i];
            }
        }
        cout<<endl;
    }

    string serializedData = "";
    for(int j = 0; j<bitArray[0].length(); j++){
        for(int i = 0; i<bitArray.size(); i++){
            serializedData += bitArray[i][j];
        }
    }

    cout<<"\ndata bits after column-wise serialization: \n";
    cout<<serializedData<<endl;

    // add check sum
    string checkSumAddedString = encode(serializedData, generatorPolynomial);

    cout<<"\ndata bits after appending CRC checkSum <send frame>:\n";
    for(int i = 0; i<checkSumAddedString.length(); i++){
        if(i >= serializedData.length())
            cout<<"\033[36m"<<checkSumAddedString[i]<<"\033[0m";
        else
            cout<<checkSumAddedString[i];
    }
    cout<<endl;

    vector<int> index = toggleBits(checkSumAddedString, toggleProbability, eng);
    string toggledString = checkSumAddedString;
    for(auto x: index){
        toggledString[x] = (toggledString[x] =='0') ? '1' : '0';
    }

    cout<<"\nreceived frame:\n";
    for(int i = 0; i<toggledString.length(); i++){
        if(find(index.begin(), index.end(), i) != index.end())
            cout<<"\033[31m"<<toggledString[i]<<"\033[0m";
        else 
            cout<<toggledString[i];
    }

    cout<<endl;

    cout<<"\nresult of CRC checksum matching: ";
    string rem = division(toggledString, generatorPolynomial);
    if(rem == "0") cout<<"no error detected\n";
    else cout<<"error detected\n";

    // remove the checkSum from the received frame
    toggledString = toggledString.substr(0, (toggledString.length() - (generatorPolynomial.length()-1)));
    
    // reuse the vector<string>bitArray
    bitArray.clear();
    
    bitArray.resize(rows, "");
    vector<pair<int, int>> errorBitTrack;   //<row, col> for tracking the position of the error bit  
    
    int colTrack = -1;
    for(int i = 0; i<toggledString.length(); i++){
        int n = i%rows;
        if(n == 0) colTrack ++;
        bitArray[n] += toggledString[i];
        if(find(index.begin(), index.end(), i) != index.end()){
            errorBitTrack.push_back(make_pair(n, colTrack));
        }
    }
    
    cout<<"\ndata block after removing CRC checksum bits:\n";
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < bitArray[i].length(); j++){
            bool flag = false;
            for(auto x: errorBitTrack){
                if(x.first == i && x.second == j){
                    cout<<"\033[31m"<<bitArray[i][j]<<"\033[0m";
                    flag = true;
                    break;
                }
            }   
            if(!flag)
                cout<<bitArray[i][j];
        }
        cout<<endl;
    }

    cout<<"\ndata blocks after removing check bits:\n";
    for(auto &x: bitArray){
        oneBitErrorCorrection(x);
        removeCheckBits(x);
        cout<<x<<endl;
    }

    cout<<endl;

    cout<<"output frame: ";
    string output = "";
    for(string x: bitArray){
        for(int i = 0; i<x.length(); i+= 8){
            string byte = x.substr(i, 8);
            char c = (char)(bitset<8>(byte).to_ulong());
            output += c;
        }
    }
    cout<<output<<endl;
    

    
  
}