#include <iostream>
#include <fstream>
#include <set>
using namespace std;

struct record {
	int iKey = -10; //to know if it empty
	int iVal = -10;
};

class sequenceSet {
	int numOfBlocks;
	int numOfRecords;
	int blockCounter = 0;
	int headerBlockLen = 6;
	int blockLen;
	int recordLen = 5;
	set<int>currentWriteBlocks;

public:
	bool CreateRecordFile(char* cIndexFile, int m, int n) {
		fstream dataFile;
		numOfBlocks = m;
		numOfRecords = n;
		blockLen = 5 * numOfRecords + numOfRecords - 1;
		try {
			dataFile.open(cIndexFile, ios::out);
			dataFile << -1 << ',' << 0 << 1 << '#';
			for (int i = 0; i < m - 1; i++) {
				if (i + 1 == m - 1) {
					dataFile << -1 << ',' << -1 << '|';
				}
				else {
					dataFile << -1 << ',' << 0 << i + 2 << '|';

				}
				for (int j = 0; j < n - 1; j++) {
					if (j == n - 2) {
						dataFile << "  " << ',' << "  ";

					}
					else {
						dataFile << "  " << ',' << "  " << '|';
					}
				}
				dataFile << '#';
			}
			blockCounter += 1;
			currentWriteBlocks.insert(blockCounter);
			return true;
		}
		catch (exception e) {
			return false;
		}
	}
	

	bool isBlockEmpty(int blockNum, char* cIndexFile) {
		char c;
		record firstBlock;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		bool isEmpty = false;
		dataFile.seekg(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1 + 6, ios::beg);
		dataFile >> c;
		if (c == ' ') {
			isEmpty = true;
		}
		return isEmpty;
	}
	

	void updateBlockLink(int blockNum, char* cIndexFile) {
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		bool isEmpty = true; 
		for (int i = blockNum+1; i < numOfBlocks; i++) {
			isEmpty = isBlockEmpty(i, cIndexFile);
			if (isEmpty == false) {
				dataFile.seekp(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1, ios::beg);
				if (i < 10) {
					dataFile << 0 << i;
				}
				else {
					dataFile << i;
				}
				break;
			}
		}
		if (isEmpty) {
			dataFile.seekp(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1, ios::beg);
			dataFile << -1;
		}
	}
	void updateBlockMaxKey(int blockNum, char* cIndexFile) {
		int maximumKey;
		bool isUpdated = false;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		record* records = readBlock(blockNum,cIndexFile);
		maximumKey = records[0].iKey;
		for (int i = 1; i < numOfRecords - 1; i++) {
			if (records[i].iKey == -10) {
				break;
			}
			if (records[i].iKey > maximumKey) {
				maximumKey = records[i].iKey;
			}
		}
		dataFile.seekp(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1 + 3, ios::beg);
		if (maximumKey == -10) {
			dataFile << -1;
		}
		else {
			if (maximumKey < 10) {
				dataFile << 0 << maximumKey;
			}
			else {
				dataFile << maximumKey;
			}
		}
	}


	void updateHeaderBlock(char* cIndexFile) {
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		bool isFileFull = true;
		bool isEmpty;
		bool isNonEmptyUpdated = false;
		bool isEmptyUpdated = false;
		dataFile.open(cIndexFile, ios::in | ios::out);
		for (int i = 1; i < numOfBlocks; i++) {// to get the first empty block
			isEmpty = isBlockEmpty(i, cIndexFile);
			if (isEmptyUpdated == false) {
				if (isEmpty) {
					dataFile.seekp(3, ios::beg);
					isEmptyUpdated = true;
					isFileFull = false;
					if (i < 10) {
						dataFile << 0 << i;
					}
					else {
						dataFile << i;
					}
				}
			}
			if (isNonEmptyUpdated == false) {
				if (isEmpty == false) {
					dataFile.seekp(0, ios::beg);
					isNonEmptyUpdated = true;
					if (i < 10) {
						dataFile << 0 << i;
					}
					else {
						dataFile << i;
					}
				}
			}

		}
		if (isFileFull) {
			dataFile.seekp(3, ios::beg);
			dataFile << -1;
		}
	}

	void splitBlock(int blockNum, char* cIndexFile) {
		int i = sizeOfBlock(blockNum, cIndexFile);
		int j = sizeOfBlock(blockNum - 1, cIndexFile);
		record* block = readBlock(blockNum, cIndexFile);
		record* previousBlock = readBlock(blockNum - 1, cIndexFile);
		if (i < ((numOfRecords - 1) / 2)) {
			block[i] = readBlock(blockNum - 1, cIndexFile)[j - 1];
			writeBlock(sortBlock(block), blockNum, cIndexFile);
			int x = (headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1) - (6 * (5 - j));
			fstream dataFile;
			dataFile.unsetf(ios::skipws);
			dataFile.open(cIndexFile, ios::in | ios::out);
			dataFile.seekp(x, ios::beg);
			dataFile << "  " << ',' << "  ";
		}
	}

	int sizeOfBlock(int blockNum, char* cIndexFile) {
		int i;
		for (i = 1; i < numOfRecords - 1; i++) {
			if (readBlock(blockNum, cIndexFile)[i].iKey == -10) {
				break;
			}
		}
		return i;
	}

	int InsertVal(char* cIndexFile, int iToken, int iKey) {
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		char input;
		dataFile >> input;
		if (input == '-') {//exception cause it is root, the file is empty
			dataFile.seekp(headerBlockLen + 3, ios::beg);
			if (iKey < 10) {
				dataFile << 0 << iKey;
			}
			else {
				dataFile << iKey;
			}
			dataFile.seekp(headerBlockLen *2, ios::beg);
			if (iKey < 10) {
				dataFile << 0 << iKey << ',';
			}
			else {
				dataFile << iKey << ',';
			}
			if (iToken < 10) {
				dataFile << 0 << iToken;
			}
			else {
				dataFile << iToken;
			}
			dataFile.close();
			updateBlockLink(blockCounter, cIndexFile);
			updateBlockMaxKey(blockCounter, cIndexFile);
			updateHeaderBlock(cIndexFile);
			return 1;

		}

		else {
			bool isInserted = false;
			for (set<int>::iterator blockIndex = currentWriteBlocks.begin(); blockIndex != currentWriteBlocks.end(); blockIndex++) {
				record* block = readBlock(*blockIndex, cIndexFile);
				for (int i = 0; i < numOfRecords - 1; i++) {
					if (block[i].iKey == -10) {// empty record, insert
						block[i].iKey = iKey;
						block[i].iVal = iToken;
						block = sortBlock(block);
						writeBlock(block, *blockIndex, cIndexFile);
						isInserted = true;
						dataFile.close();
						updateBlockLink(*blockIndex, cIndexFile);
						updateBlockMaxKey(*blockIndex, cIndexFile);
						break;
					}
				}
				if (isInserted) {
					break;
				}
			}
			if (isInserted == false) {
				blockCounter += 1;
				currentWriteBlocks.insert(blockCounter);
				dataFile.seekg(headerBlockLen + ((blockCounter - 1) * blockLen) + blockCounter - 1 + 6, ios::beg);
				if (iKey < 10) {
					dataFile << 0 << iKey << ',';
				}
				else {
					dataFile << iKey << ',';
				}
				if (iToken < 10) {
					dataFile << 0 << iToken;
				}
				else {
					dataFile << iToken;
				}
				dataFile.close();
				/*record* records = readBlock(blockCounter, cIndexFile);
				records = sortBlock(records);
				writeBlock(records, blockCounter, cIndexFile);*/
				splitBlock(blockCounter, cIndexFile);
				updateBlockLink(blockCounter-1, cIndexFile);
				updateBlockMaxKey(blockCounter, cIndexFile);
				updateBlockMaxKey(blockCounter-1, cIndexFile);
			}
		}
		updateHeaderBlock(cIndexFile);
	}

	record* readBlock(int blockNum, char* cIndexFile) {
		char c;
		record* records = new record[numOfRecords - 1];
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		int sum = 0;
		bool isEmpty = false;
		int counter = 0;
		dataFile.seekg(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1 + 6, ios::beg);
		for (int i = 0; i < numOfRecords - 1; i++) {
			while (true) {
				dataFile >> c;
				if (c == '#') {
					break;
				}
				if (c == ' ') {
					isEmpty = true;
					break;
				}
				if (c == '|') {
					counter++;
					break;
				}
				if (c == ',') {
					counter++;
					records[i].iKey = sum;
					sum = 0;
					continue;
				}
				counter++;
				if (counter == 1 || counter == 4) {// first digit multiply by 10
					sum += ((int)c - '0') * 10;
				}
				else {
					sum += (int)c - '0';
				}
			}
			if (isEmpty) {
				break;
			}
			records[i].iVal = sum;
			sum = 0;
			counter = 0;
		}
		return records;
	}
	

	void writeBlock(record* records, int blockNum, char* cIndexFile) {
		fstream dataFile;
		dataFile.open(cIndexFile, ios::in | ios::out);
		dataFile.seekp(headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1 + 6, ios::beg);
		for (int i = 0; i < numOfRecords - 1; i++) {
			if (records[i].iKey == -10) { //empty record;
				break;
			}
			if (i == numOfRecords - 2) {
				if (records[i].iKey < 10) {
					dataFile << 0 << records[i].iKey << ',';
				}
				else {
					dataFile << records[i].iKey << ',';
				}
				if (records[i].iVal < 10) {
					dataFile << 0 << records[i].iVal;
				}
				else {
					dataFile << records[i].iVal;
				}
			}
			else {
				if (records[i].iKey < 10) {
					dataFile << 0 << records[i].iKey << ',';
				}
				else {
					dataFile << records[i].iKey << ',';
				}
				if (records[i].iVal < 10) {
					dataFile << 0 << records[i].iVal << '|';
				}
				else {
					dataFile << records[i].iVal << '|';
				}
			}
		}
		dataFile.close();
	}
	

	record* sortBlock(record* records) {
		for (int i = 0; i < numOfRecords - 1; i++) {
			if (records[i].iKey == -10 || records[i + 1].iKey == -10) {
				break; // cause it is empty
			}
			for (int j = i + 1; j < numOfRecords - 1; j++) {
				if (records[j].iKey == -10) {
					break;
				}
				if (records[j].iKey < records[i].iKey) {
					record tempRecord = records[i];
					records[i] = records[j];
					records[j] = tempRecord;
				}
			}
		}
		return records;
	}

	

	/*void updateHraderRecordKey(int blockNum, char* cIndexFile) {
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in | ios::out);
		int x = headerBlockLen + ((blockNum - 1) * blockLen) + blockNum - 1;
		dataFile.seekp(x, ios::beg);
		if (blockNum == blockCounter) {
			dataFile << -1;
		}
		else {
			if (readBlock(blockNum + 1, cIndexFile)[0].iVal != -1) {
				if (blockNum < 10) {
					dataFile << 0 << blockNum + 1;
				}
				else {
					dataFile << blockNum + 1;
				}
			}
			else {
				if (blockCounter < 10) {
					dataFile << 0 << blockCounter;
				}
				else {
					dataFile << blockCounter;
				}
			}
		}
	}*/

	//void updateHeaderBlockVal(int blockNum, char* cIndexFile) {
	//	fstream dataFile;
	//	dataFile.unsetf(ios::skipws);
	//	dataFile.open(cIndexFile, ios::in | ios::out);
	//	int maxi = readBlock(blockNum, cIndexFile)[0].iKey;
	//	for (int i = 0; i < numOfRecords; i++) {
	//		if (readBlock(blockNum, cIndexFile)[i].iKey == -10) {
	//			break;
	//		}
	//		if (maxi < readBlock(blockNum, cIndexFile)[i].iKey) {
	//			maxi = readBlock(blockNum, cIndexFile)[i].iKey;
	//		}
	//	}
	//	//cout<<"maxi after  "<<maxi<<endl;
	//	int x = headerBlockLen + (blockNum - 1) * blockLen + 3 + (blockNum - 1);//6
	//	dataFile.seekp(x, ios::beg);
	//	if (maxi != 0) {
	//		if (maxi < 10) {
	//			dataFile << 0 << maxi;
	//		}
	//		else {
	//			dataFile << maxi;
	//		}
	//	}
	//	else {
	//		dataFile << -1;

	//	}
	//}
	int getKey(char* cIndexFile, int iBlock, int iRecord) {
		char c;
		int k = 0;
		iBlock--;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in);
		dataFile.seekg(headerBlockLen + ((iBlock - 1) * blockLen) + (iBlock - 1) + ((iRecord - 1) * (recordLen + 1))+6, ios::beg);
		dataFile >> c;
		if (c == ' ') {
			return -1;
		}
		if (c != '0') {
			k = ((int)c - '0')*10;
		}
		dataFile >> c;
		k += (int)c - '0';
		return k;
	}
	int getValue(char* cIndexFile, int iBlock, int iRecord) {
		char c;
		int k = 0;
		iBlock--;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in);
		dataFile.seekg(headerBlockLen + ((iBlock - 1) * blockLen) + (iBlock - 1) + ((iRecord - 1) * (recordLen + 1) + 3)+6, ios::beg);
		dataFile >> c;
		if (c == ' ') {
			return -1;
		}
		if (c != '0') {
			k = ((int)c - '0')*10;
		}
		dataFile >> c;
		k += (int)c - '0';
		return k;
	}

	int getBlockIndex(char* cIndexFile, int iToken) {
		int index = -1;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in);
		for (int block = 1; block < numOfBlocks; block++) {
			record* records = readBlock(block, cIndexFile);
			for (int rec = 0; rec < numOfRecords - 1; rec++) {
				if (records[rec].iKey == -10) {
					break;
				}
				if (records[rec].iKey == iToken) {
					index = block;
					return index;
				}
			}
		}
		return index;
	}


	int getRecordIndex(char* cIndexFile, int iToken) {
		int index = -1;
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::in);
		for (int block = 1; block < numOfBlocks; block++) {
			record* records = readBlock(block, cIndexFile);
			for (int rec = 0; rec < numOfRecords - 1; rec++) {
				if (records[rec].iKey == -10) {
					break;
				}
				if (records[rec].iKey == iToken) {
					index = rec + 1;
					return index;
				}
			}
		}
		return index;
	}

	int firstEmptyBlock(char* cIndexFile) {
		fstream dataFile;
		dataFile.open(cIndexFile, ios::in);
		char c;
		int index;
		dataFile.seekg(3, ios::beg);
		dataFile >> c;
		if (c == '-') {
			return -1;
		}
		index = (int)c - '0';
		dataFile >> c;
		index += (int)c - '0';
		return index;
	}
	void shiftBlock(char* cIndexFile, record* records, int recIndex, int blockIndex,int blockSize) {
		int counter = recIndex;
		for (counter; counter < numOfRecords - 1; counter++) {
			if (records[counter + 1].iKey == -10) {
				break;
			}
			if (counter + 1 == numOfRecords - 1) {
				continue;
			}
			else {
				records[counter] = records[counter + 1];
			}
		}

		writeBlock(records, blockIndex, cIndexFile);
		fstream dataFile;
		dataFile.open(cIndexFile, ios::in | ios::out);
		dataFile.unsetf(ios::skipws);
		dataFile.seekp(headerBlockLen + ((blockIndex - 1) * blockLen) + (blockIndex - 1) + ((blockSize-1) * (recordLen + 1))+6, ios::beg);
		dataFile << "  " << "," << "  ";
		dataFile.close();
	}
	void deleteBlock(int blockIndex, char* cIndexFile) {
		fstream dataFile;
		dataFile.unsetf(ios::skipws);
		dataFile.open(cIndexFile, ios::out | ios::in);
		dataFile.seekp(headerBlockLen + ((blockIndex - 1) * blockLen) + (blockIndex - 1));
		dataFile << -1 << ',' << -1 << '|';
		for (int j = 0; j < numOfRecords - 1; j++) {
			if (j == numOfRecords - 2) {
				dataFile << "  " << ',' << "  ";

			}
			else {
				dataFile << "  " << ',' << "  " << '|';
			}
		}
	}
	void mergeBlocks(int blockIndex1, int blockIndex2,char* cIndexfile) {
		int block1Size = sizeOfBlock(blockIndex1, cIndexfile);
		int block2Size = sizeOfBlock(blockIndex2, cIndexfile);
		record* records1 = readBlock(blockIndex1, cIndexfile);
		record* records2 = readBlock(blockIndex2, cIndexfile);
		int counter1 = 0;
		int counter2 = block2Size;
		while (true) {
			if (counter1 == block1Size) {
				break;
			}
			records2[counter2] = records1[counter1];
			counter2++;
			counter1++;
		}
		deleteBlock(blockIndex1, cIndexfile);
		writeBlock(records2, blockIndex2, cIndexfile);
	}
	void DeleteKey(char* cIndexFile, int iToken) {// delete record containing value iKey = iToken
		int blockIndex = getBlockIndex(cIndexFile, iToken);
		int recIndex = getRecordIndex(cIndexFile, iToken);
		int blockSize = sizeOfBlock(blockIndex, cIndexFile);
		fstream dataFile;
		dataFile.open(cIndexFile, ios::in | ios::out);
		dataFile.unsetf(ios::skipws);
		record* records = readBlock(blockIndex, cIndexFile);
		dataFile.seekp(headerBlockLen + ((blockIndex - 1) * blockLen) + (blockIndex - 1) + ((recIndex ) * (recordLen + 1)), ios::beg);
		dataFile << "  " << "," << "  ";
		dataFile.close();
		shiftBlock(cIndexFile, records, recIndex-1, blockIndex,blockSize);
		int sizeAfterDeleting= sizeOfBlock(blockIndex, cIndexFile);
		if (blockIndex != 1) {
			if ((sizeAfterDeleting + 1) < (int)(numOfRecords / 2) + 0.5) {
				int sizeOfBlockBefore = sizeOfBlock(blockIndex - 1, cIndexFile);
				if (sizeOfBlockBefore < (int)(numOfRecords / 2) + 0.5) {
					mergeBlocks(blockIndex, blockIndex - 1, cIndexFile);
				}
				else {
					splitBlock(blockIndex, cIndexFile);
				}
			}
			updateBlockMaxKey(blockIndex, cIndexFile);
			updateBlockLink(blockIndex - 1, cIndexFile);
			updateHeaderBlock(cIndexFile);
		}
		else {
			updateBlockMaxKey(blockIndex, cIndexFile);
			updateBlockLink(blockIndex, cIndexFile);
			updateHeaderBlock(cIndexFile);
		}
	}; 


};

int main()
{
	sequenceSet s;
	char test[20] = "test.txt";
	s.CreateRecordFile(test, 4, 5);
	fstream f;
	f.open(test, ios::in);
	char c;
	s.InsertVal(test, 10, 2);
	s.InsertVal(test, 50, 1);
	s.InsertVal(test, 10, 3);
	s.InsertVal(test, 10, 6);
	s.InsertVal(test, 50, 5);
	s.InsertVal(test, 10, 4);
	s.InsertVal(test, 10, 7);
	s.InsertVal(test, 50, 8);
	s.InsertVal(test, 10, 9);
	cout << s.getKey(test, 3, 2) << endl;
	cout << s.getValue(test, 3, 2) << endl;
	cout << s.getKey(test, 4, 1) << endl;
	cout << s.getValue(test, 4, 1) << endl;
	cout << s.getBlockIndex(test, 1);
	s.DeleteKey(test, 1);
	
}