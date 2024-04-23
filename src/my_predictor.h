// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 32 kilobytes available
// for the CBP-2 contest; it is just an example.


// my update
class my_update : public branch_update {
public:
	unsigned int index;
};




// Local Branch Predictor
class my_predictor : public branch_predictor{
public:
#define BHR_num 22 // (b.address & ((1<<PT_BITS)-1)) part of PC
#define PT_BITS 22
#define BMODAL_BITS 15  // define bimodal table bits


	unsigned char BimodalTable[1 << BMODAL_BITS];  // bimodal table

	my_update u;
	branch_info bi;
	unsigned int history;
	unsigned int BHRTable[BHR_num];
	unsigned char PTable[1<<PT_BITS];

	// initialize
	my_predictor (void) : history(0){ 
		// Initialize BHRTable with zeros
        for (int i = 0; i < BHR_num; i++) {
                BHRTable[i] = 0;
        }
        // Initialize PTable with zeros
		memset (PTable, 0, sizeof (PTable));
		// 4, mid of 0-7
    	memset(BimodalTable, 4, sizeof(BimodalTable));
	}

	branch_update *predict (branch_info &b) {
		bi = b;
		unsigned int bimodal_index = b.address & ((1 << BMODAL_BITS) - 1);
    	bool bimodal_prediction = (BimodalTable[bimodal_index] >> 1) & 1;
		if (b.br_flags & BR_CONDITIONAL) {
			u.index = 
				BHRTable[history ^ (b.address & ((1<<PT_BITS)-1))];

			bool local_prediction = (PTable[u.index] >> 1) & 1;

			if (PTable[u.index] == 3 || PTable[u.index] == 4) { // unclear
				u.direction_prediction(bimodal_prediction);
			} else {
				u.direction_prediction(local_prediction);
			}
			} else {
				u.direction_prediction(true);
			}
			u.target_prediction (0);
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target, branch_info &b) {
		if (bi.br_flags & BR_CONDITIONAL) {
			unsigned char *c = &PTable[((my_update*)u)->index];
			if (taken) {
				if (*c < 7) (*c)++;
			} else {
				if (*c > 0) (*c)--;
			}

			unsigned int bimodal_index = b.address & ((1 << BMODAL_BITS) - 1);
			if (taken) {
				if (BimodalTable[bimodal_index] < 7) BimodalTable[bimodal_index]++;
			} else {
				if (BimodalTable[bimodal_index] > 0) BimodalTable[bimodal_index]--;
			}

			BHRTable[history ^ (b.address & ((1<<PT_BITS)-1))] <<= 1;
			BHRTable[history ^ (b.address & ((1<<PT_BITS)-1))] |= taken;
			BHRTable[history ^ (b.address & ((1<<PT_BITS)-1))] &= (1<<14)-1;

			history <<= 1;
			history |= taken;
			history &= (1<<BHR_num)-1;
		}
	}


};


