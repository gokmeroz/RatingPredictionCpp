#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <cmath>
#include <unordered_map>
using namespace std;

class Prediction {
public:
    //this constructor does all the job when an object created
    Prediction(const string &trainpath, const string &testpath, const string &resultpath) {
        ReadData(trainpath);//to read the traim.csv's data
        Write(testpath, resultpath);//to write on result.csv
    }

    unordered_map<int, map<int, float>> UsersList;//an unordered-map that stores an integer and an UsersList type
    unordered_map<int, map<int, float>> ItemsList;//an unordered-map that stores an integer and an ItemsList type

    //that function is a representation of cosine similarity
    float similarity(vector<float> &vector1, vector<float> &vector2) {
        float result = 0, col1 = 0, col2 = 0;
        for (int i = 0; i < vector1.size(); ++i) {
            result += vector1[i] * vector2[i];
            col1 += vector1[i] * vector1[i];
            col2 += vector2[i] * vector2[i];
        }
        return result / (sqrt(col1) * sqrt(col2));
    }
    //that function reads the train.csv's data
    void ReadData(const string &trainpath) {
        ifstream trainfile;
        trainfile.open(trainpath);
        string line;              // all the data in trainfile and testfile are string eventhought they seem like numbers, we will read strings
        getline(trainfile,line); // the first column in traifile is useless, because of that we need to skip the first line
        while (getline(trainfile, line)) {

            stringstream ss(line);
            string userid, itemid, rating;
            getline(ss, userid,','); // untill sees the first comma symbol it reads the column and initilaze the data as userid
            getline(ss, itemid,','); // untill sees the second comma symbol it reads the column and initilaze the data as itemid
            getline(ss, rating);      // untill the rest of the column and initilaze the data as rating
            UsersList[stoi(userid)][stoi(itemid)] = stof(rating);//we store the ratings for users in UsersList
            ItemsList[stoi(itemid)][stoi(userid)] = stof(rating);//we store the rating for items in ItemsList
        }
    }
    //that function calculates the ubcf similarity to find the most accurate rating with ubcf
    float SimilarityUBCF(int user1, int user2) {
        vector<float> v1, v2;//these two vectors will be sent in similarity() function
        map<int, float> user1map = UsersList[user1];//this user1map is getting its values from the UsersList
        map<int, float> user2map = UsersList[user2];//this user2map is getting its values from the UsersList
        //iterator is usefull to travelling inside the map, and it does start from the beginning of the user1map(and user2map)
        map<int, float>::iterator i1 = user1map.begin();
        map<int, float>::iterator i2 = user2map.begin();

        //while loop does its job untill the both maps have literally no value inside them
        while (i1 != user1map.end() && i2 != user2map.end()) {
           //checks if the userid's are the same
            if (i1->first == i2->first) {
                v1.push_back(i1->second);//store the float values inside the v1 vector
                v2.push_back(i2->second);//store the float values inside the v2 vector
                i1++;//to check the other userid
                i2++;//to check the other userid
            } else if (i1->first < i2->first) i1++;
            else i2++;
        }
        return similarity(v1, v2);//send the vectors into the similarity() to check the cosine value
    }
    //that function calculates the ibcf similarity to find the most accurate rating with ibcf
    //does the same thing just like the SimilarityUBCF()
    float SimilarityIBCF(int item1, int item2) {
        vector<float> v1, v2;
        map<int, float> item1map = ItemsList[item1];
        map<int, float> item2map = ItemsList[item2];
        map<int, float>::iterator i1 = item1map.begin();
        map<int, float>::iterator i2 = item2map.begin();

        while (i1 != item1map.end() && i2 != item2map.end()) {
            if (i1->first == i2->first) {
                v1.push_back(i1->second);
                v2.push_back(i2->second);
                i1++;
                i2++;
            } else if (i1->first < i2->first) i1++;
            else i2++;
        }
        return similarity(v1, v2);
    }
    //to calculate the rating via ubcf
    float CalculateerIBCF(int userid,int itemid){

            map<int, float> usermap = UsersList[userid];//this usermap is getting its values from the UsersList
            map<int, float>::iterator i = usermap.begin();//the iterator is usefull to travell inside the usermap

        float rating = 0;
        int n = 0;

        //while loop does its job untill the usermap does not have any other value to check
        while (i != usermap.end()) {
            float s = SimilarityIBCF(itemid, i->first);//s has the similarityIBCF's value for itemid
            //this piece of code is to faster the loop because its only checks for the most similar ones (0.90)
            if (s > 0.916) {
                rating += i->second;
                n++;
            }
            i++;
        }
        //to not getting nan this piece of code is declare the raitng as 3.75
        if(n==0){
            return 3.82;
        }
        return rating / n;//total rating value/number of that item
        }
    //to calculate the rating via ibcf, it does the same thing as CalculaterIBCF
    float CalculaterUBFC(int userid, int itemid) {
        map<int, float> itemmap = ItemsList[itemid];
        map<int, float>::iterator i = itemmap.begin();

        float rating = 0;
        int n = 0;

        while (i != itemmap.end()) {
            float s = SimilarityUBCF(userid, i->first);
            if (s > 0.916) {
                rating += i->second;
                n++;
            }
            i++;
        }
        if(n==0){
            return 3.82;
        }
        return rating / n;

    }
    //to writes the results
    void Write(const string &testpath, const string &resultpath) {
        string line;
        ifstream testfile;
        testfile.open(testpath);
        ofstream result;
        result.open(resultpath);
        getline(testfile,line); // the first column in traindata is useless because of that we need to skip the first line
        result << "ID,Predicted" << endl;
        while (getline(testfile, line)) {
            stringstream ss(line);
            string id, userid, itemid;
            getline(ss, id,','); // untill sees the first comma symbol it reads the column and initilaze the data as userid
            getline(ss, userid,','); // untill sees the second comma symbol it reads the column and initilaze the data as itemid
            getline(ss, itemid); // untill the rest of the column and initilaze the data as rating

            //to use the ubcf and ibcf together, this variable takes both the results from CalculaterUBCF() and CalculaterIBCF()
            //add them eachothers and divided by 2
           float rating = (CalculaterUBFC(stoi(userid), stoi(itemid))
                   +
                   CalculateerIBCF(stoi(userid),stoi(itemid)))
                   /2;
           result << id << "," << rating << endl;//ID,rating\n
        }
    }
    //to print top10 users and top10 items
    void Top10(const string &trainpath)
    {

            map<string, int> itemCount;//to store the every single itemid
            map<string, int> userCount;//to store every single userid

            ifstream trainfile;
            trainfile.open(trainpath);
            string line;              // all the data in trainfile and testfile are string eventhought they seem number we will read strings
            getline(trainfile,line); // the first column in traindata is useless because of that we need to skip the first line
          //reads the train.csv
            while (getline(trainfile, line))
            {

                stringstream ss(line);
                string userid, itemid, rating;
                getline(ss, userid,','); // untill sees the first comma symbol it reads the column and initilaze the data as userid
                getline(ss, itemid,','); // untill sees the second comma symbol it reads the column and initilaze the data as itemid
                getline(ss, rating);      // untill the rest of the column and initilaze the data as rating
                userCount[userid]++;//when it does read an userid which is already in the map it add +1 to the value of that map
                itemCount[itemid]++;//when it does read an itemid which is already in the map it add +1 to the value of that map
            }

            // for the top10 items
            vector<pair<string, int>> top10Items;//this vector will store top10Items
            for (const auto &[itemid, count]: itemCount)
            {
                top10Items.push_back({itemid, count});
                // to sort the top10Items
                sort(top10Items.begin(), top10Items.end(),
                     [](const auto &a, const auto &b) { return a.second > b.second; });
                // to find the top10Items this loop pops for 10 times
                if (top10Items.size() > 10) top10Items.pop_back();
            }
            //to print top10 items
            cout << "Top 10 items:" << endl;
            for (const auto &[itemid, count]: top10Items)
            {
                cout << itemid << ": " << count << " votes" << endl;
            }
            cout << endl;
            // does the same thing just like the top10Items but only does it for top10users
            //the rest is the same
            vector<pair<string, int>> top10Users;
            for (const auto &[userid, rating]: userCount)
            {
                top10Users.push_back({userid, rating});

                sort(top10Users.begin(), top10Users.end(),
                     [](const auto &a, const auto &b) { return a.second > b.second; });

                if (top10Users.size() > 10) top10Users.pop_back();
            }

            cout << "Top 10 users:" << endl;
            for (const auto &[userid, rating]: top10Users)
            {
                cout << userid << ": " << rating << " points" << endl;
            }
    }
};

int main()
{
    Prediction p("train.csv","test.csv","result.csv");//when an object,in this case p, creates
    // it automatically start print the predicted ratings into the result.csv
    p.Top10("train.csv");//to prints the top 10 users and top 10 items

    return 0;
}