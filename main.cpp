#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <ctime>

using namespace std;

struct Booking;
struct User;
struct Flight;

int GLOBAL_BOOKING_ID = 1000;
int GLOBAL_USER_ID = 2000;

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string readLine(const string &prompt = "") {
    if (!prompt.empty()) cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\nInput stream closed. Exiting.\n";
        exit(0);
    }
    return s;
}

int readInt(const string &prompt = "") {
    while (true) {
        string s = readLine(prompt);
        s = trim(s);
        if (s.empty()) {
            cout << "Enter a number.\n";
            continue;
        }
        try {
            size_t idx = 0;
            int val = stoi(s, &idx);
            if (idx != s.size()) throw runtime_error("extra chars");
            return val;
        } catch (...) {
            cout << "Invalid number, try again.\n";
        }
    }
}

struct LinkedWaitNode {
    int userId;
    int passengers;
    string requestedClass;
    LinkedWaitNode* next;
    LinkedWaitNode(int uid,int p,const string& c)
        :userId(uid),passengers(p),requestedClass(c),next(nullptr){}
};

struct WaitList {
    LinkedWaitNode* head=nullptr;
    void push(int uid,int p,const string &c){
        LinkedWaitNode* n=new LinkedWaitNode(uid,p,c);
        if(!head){ head=n; return; }
        LinkedWaitNode* t=head;
        while(t->next) t=t->next;
        t->next=n;
    }
    bool pop_front(int &uid,int &p,string &c){
        if(!head) return false;
        LinkedWaitNode* t=head;
        head=head->next;
        uid=t->userId; p=t->passengers; c=t->requestedClass;
        delete t;
        return true;
    }
    bool empty(){ return head==nullptr; }
    vector<tuple<int,int,string>> dump(){
        vector<tuple<int,int,string>> out;
        auto t=head;
        while(t){
            out.emplace_back(t->userId,t->passengers,t->requestedClass);
            t=t->next;
        }
        return out;
    }
};

struct Booking {
    int bookingId;
    int userId;
    int flightNo;
    string seat;
    string status;
    string bookedClass;
    string bookingTime;
    int passengers;
    Booking(){}
    Booking(int b,int uid,int f,const string& s,const string& st,
            const string& bc,const string &bt,int p)
    :bookingId(b),userId(uid),flightNo(f),seat(s),status(st),
     bookedClass(bc),bookingTime(bt),passengers(p){}
};

struct Flight {
    int flightNo;
    string airline;
    string origin;
    string destination;
    string depart;
    string arrive;
    int capacity;
    int rows;
    int cols;
    vector<string> seats;
    vector<int> seatTakenByBooking;
    int fareEconomy;
    int fareBusiness;
    WaitList waitlist;
    int bookingsCount;
    Flight(){}
    Flight(int fn,const string& al,const string& o,const string& d,
           const string& dep,const string& arr,int cap,int r,int c,
           int fe,int fb)
    :flightNo(fn),airline(al),origin(o),destination(d),depart(dep),arrive(arr),
     capacity(cap),rows(r),cols(c),fareEconomy(fe),fareBusiness(fb),bookingsCount(0)
    {
        seats.resize(capacity);
        seatTakenByBooking.assign(capacity,0);
        for(int i=0;i<capacity;i++){
            int row = i/cols + 1;
            char colc = 'A' + (i%cols);
            seats[i] = to_string(row) + string(1,colc);
        }
    }
    int freeSeats(){
        int c=0;
        for(int i=0;i<capacity;i++) if(seatTakenByBooking[i]==0) c++;
        return c;
    }
    int bestAvailable(){
        for(int i=0;i<capacity;i++) if(seatTakenByBooking[i]==0) return i;
        return -1;
    }
    bool assignSeatByIndex(int idx,int bookingId){
        if(idx<0||idx>=capacity) return false;
        if(seatTakenByBooking[idx]!=0) return false;
        seatTakenByBooking[idx]=bookingId;
        bookingsCount++;
        return true;
    }
    bool freeSeatByBooking(int bookingId){
        for(int i=0;i<capacity;i++){
            if(seatTakenByBooking[i]==bookingId){
                seatTakenByBooking[i]=0;
                bookingsCount--;
                return true;
            }
        }
        return false;
    }
    string seatLabel(int idx){
        if(idx<0||idx>=capacity) return "";
        return seats[idx];
    }
    int seatIndexByLabel(const string &lbl){
        for(int i=0;i<capacity;i++) if(seats[i]==lbl) return i;
        return -1;
    }
};

struct FlightNode {
    Flight data;
    FlightNode* left;
    FlightNode* right;
    FlightNode(const Flight &f):data(f),left(nullptr),right(nullptr){}
};

struct FlightBST {
    FlightNode* root=nullptr;
    FlightNode* insertNode(FlightNode* node,const Flight &f){
        if(!node) return new FlightNode(f);
        if(f.flightNo < node->data.flightNo) node->left = insertNode(node->left,f);
        else if(f.flightNo > node->data.flightNo) node->right = insertNode(node->right,f);
        else node->data = f;
        return node;
    }
    void insert(const Flight &f){ root = insertNode(root,f); }
    FlightNode* findNode(FlightNode* node,int fn){
        if(!node) return nullptr;
        if(fn==node->data.flightNo) return node;
        if(fn < node->data.flightNo) return findNode(node->left,fn);
        return findNode(node->right,fn);
    }
    Flight* find(int fn){
        auto n=findNode(root,fn);
        if(!n) return nullptr;
        return &(n->data);
    }
    FlightNode* minNode(FlightNode* node){
        while(node && node->left) node=node->left;
        return node;
    }
    FlightNode* removeNode(FlightNode* node,int fn){
        if(!node) return node;
        if(fn < node->data.flightNo) node->left = removeNode(node->left,fn);
        else if(fn > node->data.flightNo) node->right = removeNode(node->right,fn);
        else {
            if(!node->left){
                FlightNode* r=node->right;
                delete node;
                return r;
            } else if(!node->right){
                FlightNode* l=node->left;
                delete node;
                return l;
            } else {
                FlightNode* succ = minNode(node->right);
                node->data = succ->data;
                node->right = removeNode(node->right,succ->data.flightNo);
            }
        }
        return node;
    }
    void remove(int fn){ root = removeNode(root,fn); }
    void inorderNode(FlightNode* node, vector<Flight*> &acc){
        if(!node) return;
        inorderNode(node->left,acc);
        acc.push_back(&node->data);
        inorderNode(node->right,acc);
    }
    vector<Flight*> listAll(){
        vector<Flight*> acc;
        inorderNode(root,acc);
        return acc;
    }
};

struct User {
    int userId;
    string username;
    string password;
    bool isAdmin;
    string fullName;
    string email;
    int loyaltyPoints;
    vector<int> myBookings;
    User(){}
    User(int uid,const string& un,const string& pw,bool adm,
         const string& fn,const string& em)
    :userId(uid),username(un),password(pw),isAdmin(adm),
     fullName(fn),email(em),loyaltyPoints(0){}
};

struct System {
    FlightBST flights;
    unordered_map<int,User> users;
    unordered_map<int,Booking> bookings;
    unordered_map<string,int> userByUsername;

    System(){
        loadFromDisk();
        if(users.empty()){
            User admin(GLOBAL_USER_ID++,"admin","admin123",true,
                       "System Admin","admin@system");
            users[admin.userId]=admin;
            userByUsername[admin.username]=admin.userId;
        }
    }
    ~System(){ saveToDisk(); }

    string nowStr(){
        time_t t = time(nullptr);
        char buf[64];
        strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",localtime(&t));
        return string(buf);
    }

    int registerUser(const string &username,const string &password,
                     const string &fullName,const string &email){
        if(userByUsername.count(username)) return -1;
        int id = GLOBAL_USER_ID++;
        User u(id,username,password,false,fullName,email);
        users[id]=u;
        userByUsername[username]=id;
        saveToDisk();
        return id;
    }

    User* login(const string &username,const string &password){
        if(!userByUsername.count(username)) return nullptr;
        int id = userByUsername[username];
        if(users[id].password==password) return &users[id];
        return nullptr;
    }

    bool addFlight(const Flight &f){
        if(flights.find(f.flightNo)) return false;
        flights.insert(f);
        saveToDisk();
        return true;
    }

    bool removeFlight(int fn){
        Flight* f = flights.find(fn);
        if(!f) return false;
        if(f->bookingsCount>0) return false;
        flights.remove(fn);
        saveToDisk();
        return true;
    }

    vector<Flight*> searchFlights(const string &origin,const string &destination,
                                  const string &date){
        vector<Flight*> out;
        auto all = flights.listAll();
        for(auto f: all){
            if( (origin=="" || f->origin==origin) &&
                (destination=="" || f->destination==destination) ){
                if(date=="" || f->depart.find(date)!=string::npos)
                    out.push_back(f);
            }
        }
        return out;
    }

    int createBooking(int userId,int flightNo,const string &seatPref,
                      const string &classType,int passengers){
        Flight* f = flights.find(flightNo);
        if(!f) return -1;
        if(passengers <= 0) return -1;

        if(f->freeSeats() < passengers){
            f->waitlist.push(userId,passengers,classType);
            saveToDisk();
            return -2;
        }

        int bId = GLOBAL_BOOKING_ID++;
        vector<int> assignedIdxs;

        if(trim(seatPref)!="" && trim(seatPref)!="ANY"){
            int idx = f->seatIndexByLabel(trim(seatPref));
            if(idx!=-1 && f->seatTakenByBooking[idx]==0){
                f->assignSeatByIndex(idx,bId);
                assignedIdxs.push_back(idx);
            }
        }

        while((int)assignedIdxs.size() < passengers){
            int idx = f->bestAvailable();
            if(idx==-1) break;
            f->assignSeatByIndex(idx,bId);
            assignedIdxs.push_back(idx);
        }

        string seatLabelAll="";
        for(size_t i=0;i<assignedIdxs.size();i++){
            if(i) seatLabelAll += ",";
            seatLabelAll += f->seatLabel(assignedIdxs[i]);
        }

        Booking bk(bId,userId,flightNo,seatLabelAll,"CONFIRMED",
                   classType,nowStr(),passengers);
        bookings[bId]=bk;
        users[userId].myBookings.push_back(bId);
        users[userId].loyaltyPoints += 10*passengers;
        saveToDisk();
        return bId;
    }

    bool cancelBooking(int userId,int bookingId){
        if(!bookings.count(bookingId)) return false;
        Booking &bk = bookings[bookingId];
        if(bk.userId != userId) return false;
        Flight* f = flights.find(bk.flightNo);
        if(!f) return false;

        f->freeSeatByBooking(bookingId);
        bk.status = "CANCELLED";

        auto &vec = users[userId].myBookings;
        vec.erase(remove(vec.begin(),vec.end(),bookingId),vec.end());
        users[userId].loyaltyPoints =
            max(0, users[userId].loyaltyPoints - 5*bk.passengers);

        int wid,wp; string wc;
        if(f->waitlist.pop_front(wid,wp,wc)){
            createBooking(wid,f->flightNo,"ANY",wc,wp);
        }
        saveToDisk();
        return true;
    }

    vector<Booking> getUserBookings(int userId){
        vector<Booking> out;
        if(!users.count(userId)) return out;
        for(int b : users[userId].myBookings)
            if(bookings.count(b)) out.push_back(bookings[b]);
        return out;
    }

    vector<Booking> getAllBookings(){
        vector<Booking> out;
        for(auto &p: bookings) out.push_back(p.second);
        return out;
    }

    void saveToDisk(){
        ofstream fu("flights.dat");
        if(fu){
            auto all = flights.listAll();
            for(auto f: all){
                fu << f->flightNo << '|' << f->airline << '|' << f->origin
                   << '|' << f->destination << '|' << f->depart << '|'
                   << f->arrive << '|' << f->capacity << '|' << f->rows
                   << '|' << f->cols << '|' << f->fareEconomy << '|'
                   << f->fareBusiness << '|' << f->bookingsCount << '\n';
                fu << "SEATS|";
                for(int i=0;i<f->capacity;i++){
                    fu << f->seats[i] << ',' << f->seatTakenByBooking[i] << ';';
                }
                fu << '\n';
                auto wl = f->waitlist.dump();
                fu << "WAIT|";
                for(auto &t:wl)
                    fu << get<0>(t) << ',' << get<1>(t) << ',' << get<2>(t) << ';';
                fu << '\n';
            }
        }

        ofstream fu2("users.dat");
        if(fu2){
            for(auto &p: users){
                User &u = p.second;
                fu2 << u.userId << '|' << u.username << '|' << u.password
                    << '|' << u.isAdmin << '|' << u.fullName << '|'
                    << u.email << '|' << u.loyaltyPoints << '|';
                for(size_t i=0;i<u.myBookings.size();i++){
                    if(i) fu2<<',';
                    fu2<<u.myBookings[i];
                }
                fu2 << '\n';
            }
        }

        ofstream fb("bookings.dat");
        if(fb){
            for(auto &p: bookings){
                Booking &b = p.second;
                fb << b.bookingId << '|' << b.userId << '|' << b.flightNo
                   << '|' << b.seat << '|' << b.status << '|' << b.bookedClass
                   << '|' << b.bookingTime << '|' << b.passengers << '\n';
            }
        }
    }

    void loadFromDisk(){
        flights.root = nullptr;
        users.clear();
        bookings.clear();
        userByUsername.clear();

        ifstream fu("users.dat");
        if(fu){
            string line;
            while(getline(fu,line)){
                if(trim(line).empty()) continue;
                stringstream ss(line);
                string token;
                vector<string> parts;
                while(getline(ss,token,'|')) parts.push_back(token);
                if(parts.size()>=7){
                    int uid = stoi(parts[0]);
                    string uname = parts[1];
                    string pw = parts[2];
                    bool adm = (parts[3]=="1" || parts[3]=="true");
                    string fn = parts[4];
                    string em = parts[5];
                    int lp = stoi(parts[6]);
                    User u(uid,uname,pw,adm,fn,em);
                    u.loyaltyPoints = lp;
                    if(parts.size()>7 && !parts[7].empty()){
                        stringstream bb(parts[7]);
                        string b;
                        while(getline(bb,b,',')) u.myBookings.push_back(stoi(b));
                    }
                    users[uid]=u;
                    userByUsername[uname]=uid;
                    GLOBAL_USER_ID = max(GLOBAL_USER_ID, uid+1);
                }
            }
        }

        ifstream ff("flights.dat");
        if(ff){
            string line;
            Flight* lastFlight = nullptr;
            while(getline(ff,line)){
                string tline = trim(line);
                if(tline.empty()) continue;

                if(tline.rfind("SEATS|",0)==0){
                    if(!lastFlight) continue;
                    string rest = tline.substr(6);
                    vector<string> items;
                    string tmp;
                    stringstream ss2(rest);
                    while(getline(ss2,tmp,';'))
                        if(!trim(tmp).empty()) items.push_back(tmp);
                    int i=0;
                    for(auto &it: items){
                        stringstream s3(it);
                        string lbl; int bookingId;
                        if(getline(s3,lbl,',') && s3>>bookingId){
                            if(i < lastFlight->capacity){
                                lastFlight->seats[i] = lbl;
                                lastFlight->seatTakenByBooking[i] = bookingId;
                            }
                        }
                        i++;
                    }
                    continue;
                }

                if(tline.rfind("WAIT|",0)==0){
                    if(!lastFlight) continue;
                    string rest=tline.substr(5);
                    vector<string> items;
                    string tmp;
                    stringstream ss3(rest);
                    while(getline(ss3,tmp,';'))
                        if(!trim(tmp).empty()) items.push_back(tmp);
                    for(auto &it: items){
                        stringstream s4(it);
                        int uid,p; string cl;
                        string a;
                        if(getline(s4,a,',')) uid=stoi(a);
                        if(getline(s4,a,',')) p=stoi(a);
                        if(getline(s4,cl,',')) lastFlight->waitlist.push(uid,p,cl);
                    }
                    continue;
                }

                stringstream ss(tline);
                vector<string> parts;
                string token;
                while(getline(ss,token,'|')) parts.push_back(token);
                if(parts.size()>=11){
                    int fn=stoi(parts[0]);
                    string al=parts[1], o=parts[2], d=parts[3],
                           dep=parts[4], arr=parts[5];
                    int cap=stoi(parts[6]);
                    int r=stoi(parts[7]), c=stoi(parts[8]),
                        fe=stoi(parts[9]), fb=stoi(parts[10]);
                    Flight f(fn,al,o,d,dep,arr,cap,r,c,fe,fb);
                    if(parts.size()>11) f.bookingsCount = stoi(parts[11]);
                    flights.insert(f);
                    auto all = flights.listAll();
                    lastFlight = all.back();
                }
            }
        }

        ifstream fb("bookings.dat");
        if(fb){
            string line;
            while(getline(fb,line)){
                if(trim(line).empty()) continue;
                stringstream ss(line);
                vector<string> parts;
                string token;
                while(getline(ss,token,'|')) parts.push_back(token);
                if(parts.size()>=8){
                    Booking b;
                    b.bookingId = stoi(parts[0]);
                    b.userId = stoi(parts[1]);
                    b.flightNo = stoi(parts[2]);
                    b.seat = parts[3];
                    b.status = parts[4];
                    b.bookedClass = parts[5];
                    b.bookingTime = parts[6];
                    b.passengers = stoi(parts[7]);
                    bookings[b.bookingId]=b;
                    GLOBAL_BOOKING_ID = max(GLOBAL_BOOKING_ID, b.bookingId+1);
                }
            }
        }
    }
} sys;

void showFlightDetailed(Flight* f){
    cout<<"Flight "<<f->flightNo<<" | "<<f->airline<<" "
        <<f->origin<<"->"<<f->destination<<"\nDep:"<<f->depart
        <<" Arr:"<<f->arrive<<" Cap:"<<f->capacity<<" Free:"
        <<f->freeSeats()<<"\nFares - Econ:"<<f->fareEconomy
        <<" Bus:"<<f->fareBusiness<<"\nSeat map (label:bookingId):\n";
    for(int i=0;i<f->capacity;i++){
        cout<<f->seats[i]<<":"
            <<(f->seatTakenByBooking[i]==0?"free":to_string(f->seatTakenByBooking[i]))
            <<" ";
        if( (i+1)%f->cols==0 ) cout<<"\n";
    }
    cout<<"\nWaitlist entries: ";
    auto wl = f->waitlist.dump();
    cout<<wl.size()<<"\n";
}

void adminMenu(User* admin){
    while(true){
        cout << "\n--- ADMIN MENU ---\n"
             << "1) Add Flight\n"
             << "2) Edit Flight\n"
             << "3) Remove Flight\n"
             << "4) View All Flights\n"
             << "5) View All Bookings\n"
             << "6) Force Cancel Booking\n"
             << "7) View Users\n"
             << "8) Export Report (text)\n"
             << "9) Logout\n";
        int ch = readInt("Choose: ");

        if(ch==1){
            int fn = readInt("Flight No: ");
            string airline = readLine("Airline: ");
            string org = readLine("Origin: ");
            string dest = readLine("Destination: ");
            string dep = readLine("Departure(datetime): ");
            string arr = readLine("Arrival(datetime): ");
            int r = readInt("Rows: ");
            int c = readInt("Cols: ");
            int cap = r*c;
            int fe = readInt("Fare Economy: ");
            int fb = readInt("Fare Business: ");
            Flight f(fn,airline,org,dest,dep,arr,cap,r,c,fe,fb);
            if(sys.addFlight(f)) cout<<"Added flight "<<fn<<"\n";
            else cout<<"Flight exists or error\n";
        } else if(ch==2){
            int fn = readInt("Enter Flight No to edit: ");
            Flight* f = sys.flights.find(fn);
            if(!f){ cout<<"Flight not found\n"; continue; }
            cout<<"1) Update Times\n2) Update Fares\n3) Update Route\n4) Show Details\n";
            int c2 = readInt("Choose: ");
            if(c2==1){
                string d = readLine("New Departure: ");
                string a = readLine("New Arrival: ");
                f->depart=d; f->arrive=a;
                cout<<"Updated\n";
            } else if(c2==2){
                int fe = readInt("New Econ fare: ");
                int fb = readInt("New Bus fare: ");
                f->fareEconomy=fe; f->fareBusiness=fb;
            } else if(c2==3){
                string o = readLine("New Origin: ");
                string de = readLine("New Destination: ");
                f->origin=o; f->destination=de;
            } else {
                showFlightDetailed(f);
            }
            sys.saveToDisk();
        } else if(ch==3){
            int fn = readInt("Enter Flight No to remove: ");
            if(sys.removeFlight(fn)) cout<<"Removed\n";
            else cout<<"Cannot remove (maybe bookings exist) or not found\n";
        } else if(ch==4){
            auto all = sys.flights.listAll();
            cout<<"All Flights ("<<all.size()<<"):\n";
            for(auto f: all){
                cout<<f->flightNo<<" | "<<f->airline<<" | "<<f->origin
                    <<"->"<<f->destination<<" | Dep:"<<f->depart
                    <<" | SeatsFree:"<<f->freeSeats()
                    <<" | Waitlist items: "<<f->waitlist.dump().size()<<"\n";
            }
        } else if(ch==5){
            auto allb = sys.getAllBookings();
            cout<<"All Bookings:\n";
            for(auto &b: allb){
                cout<<b.bookingId<<" UID:"<<b.userId<<" FL:"<<b.flightNo
                    <<" Seats:"<<b.seat<<" Status:"<<b.status
                    <<" Time:"<<b.bookingTime<<"\n";
            }
        } else if(ch==6){
            int bid = readInt("Booking ID to cancel: ");
            if(!sys.bookings.count(bid)){ cout<<"Not found\n"; continue; }
            int uid = sys.bookings[bid].userId;
            if(sys.cancelBooking(uid,bid)) cout<<"Cancelled\n";
            else cout<<"Failed\n";
        } else if(ch==7){
            cout<<"Users:\n";
            for(auto &p: sys.users){
                cout<<p.second.userId<<" | "<<p.second.username<<" | "
                    <<p.second.fullName<<" | admin:"<<p.second.isAdmin
                    <<" | points:"<<p.second.loyaltyPoints
                    <<" bookings:"<<p.second.myBookings.size()<<"\n";
            }
        } else if(ch==8){
            ofstream r("report.txt");
            auto all = sys.flights.listAll();
            r<<"Flight Report generated at "<<sys.nowStr()<<"\n";
            for(auto f:all){
                r<<"Flight "<<f->flightNo<<" "<<f->airline<<" "
                  <<f->origin<<"->"<<f->destination<<" Dep:"<<f->depart
                  <<" SeatsFree:"<<f->freeSeats()<<"\n";
            }
            r.close();
            cout<<"Report exported to report.txt\n";
        } else if(ch==9) break;
        else cout<<"Invalid\n";
    }
}

void userMenu(User* u){
    while(true){
        cout<<"\n--- USER MENU ("<<u->username<<") ---\n"
            <<"1) Search Flights\n"
            <<"2) Book Flight\n"
            <<"3) Cancel Booking\n"
            <<"4) View My Bookings\n"
            <<"5) View Profile\n"
            <<"6) Logout\n";
        int ch = readInt("Choose: ");

        if(ch==1){
            string o = readLine("Origin (enter for any): ");
            string d = readLine("Destination (enter for any): ");
            string dt = readLine("Date (YYYY-MM-DD or enter for any): ");
            auto res = sys.searchFlights(trim(o),trim(d),trim(dt));
            cout<<"Found "<<res.size()<<"\n";
            for(auto f:res) showFlightDetailed(f);
        } else if(ch==2){
            int fn = readInt("Flight No to book: ");
            Flight* f = sys.flights.find(fn);
            if(!f){ cout<<"No such flight\n"; continue; }
            int pc = readInt("Passengers count: ");
            string sp = readLine("Seat preference (e.g. 1A or ANY): ");
            string cl = readLine("Class (Economy/Business): ");
            int res = sys.createBooking(u->userId,fn,trim(sp),trim(cl),pc);
            if(res==-1) cout<<"Error creating booking\n";
            else if(res==-2) cout<<"Not enough seats, added to waitlist\n";
            else cout<<"Booking created. ID: "<<res<<"\n";
        } else if(ch==3){
            cout<<"Your bookings:\n";
            auto bs = sys.getUserBookings(u->userId);
            for(auto &b:bs)
                cout<<b.bookingId<<" FL:"<<b.flightNo<<" Seats:"<<b.seat
                    <<" Status:"<<b.status<<"\n";
            int bid = readInt("Booking ID to cancel: ");
            if(sys.cancelBooking(u->userId,bid)) cout<<"Cancelled\n";
            else cout<<"Failed\n";
        } else if(ch==4){
            auto bs = sys.getUserBookings(u->userId);
            if(bs.empty()) cout<<"No bookings\n";
            for(auto &b: bs){
                cout<<b.bookingId<<" FL:"<<b.flightNo<<" Seats:"<<b.seat
                    <<" Status:"<<b.status<<" Time:"<<b.bookingTime<<"\n";
            }
        } else if(ch==5){
            cout<<"Profile: "<<u->fullName<<" ("<<u->username<<")\n"
                <<"Email: "<<u->email<<"\n"
                <<"Loyalty Points: "<<u->loyaltyPoints<<"\n"
                <<"Bookings: "<<u->myBookings.size()<<"\n";
        } else if(ch==6) break;
        else cout<<"Invalid\n";
    }
}

void welcomeScreen(){
    while(true){
        cout<<"\n=== FLIGHT MANAGEMENT SYSTEM ===\n"
            <<"1) Login\n"
            <<"2) Register\n"
            <<"3) Exit\n";
        int ch = readInt("Choose: ");

        if(ch==1){
            string uname = readLine("Username: ");
            string pw = readLine("Password: ");
            User* u = sys.login(uname,pw);
            if(!u) cout<<"Invalid credentials\n";
            else {
                if(u->isAdmin) adminMenu(u);
                else userMenu(u);
            }
        } else if(ch==2){
            string uname = readLine("Choose username: ");
            string pw = readLine("Password: ");
            string fn = readLine("Full name: ");
            string email = readLine("Email: ");
            int id = sys.registerUser(uname,pw,fn,email);
            if(id==-1) cout<<"Username exists\n";
            else cout<<"Registered (UID "<<id<<")\n";
        } else if(ch==3){
            cout<<"Bye\n";
            break;
        } else cout<<"Invalid\n";
    }
}

int main(){
    cout<<"Starting Flight Management System...\n";
    welcomeScreen();
    return 0;
}
