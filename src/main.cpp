#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "SupportPoint.h"
#include "Package.h"

#include "graphviewer.h"
#include "SupportPoint.h"
#include "Point.h"
#include "Road.h"
#include "MutablePriorityQueue.h"
#ifdef linux
#else
#include <Windows.h>
#endif
void menuUser();
void menuBase();

struct nodeEdge {
    std::vector<Point *> points;
    std::vector<Road *> roads;
    int lenght;

} typedef nodeEdge_t;

bool compSL(nodeEdge_t i,nodeEdge_t j) {
    return i.lenght < j.lenght;
}

GraphViewer *gv;
nodeEdge_t mainMap;
vector<Vehicle> Fleet;
vector<vector<Package>> PackagesToDelivery;
Point * centralPoint;

void menuControler();


Point * findPoint(int id) {

    for(auto p : mainMap.points) {
        if(p->getID() == id) {
            return p;
        }
    }
    cout << "Point not found\n";
    return nullptr;
}

void readMap(string cityName) {

    ifstream file;

    string nodeFile = "T09/" + cityName + "/T09_nodes_X_Y_" + cityName + ".txt";
    file.open(nodeFile);

    if (!file) {
        cout << "Failed to open node file\n";
        exit(1);
    }

    string line;
    int temp;
    double id, x, y;
    int xOffset = 0, yOffset = 0;

    getline(file, line);
    sscanf(line.c_str(), "%d", &temp); //numNos
    unsigned int numNodes = temp;
    while (getline(file, line) && temp != 0) {
        sscanf(line.c_str(), "(%lf, %lf, %lf)", &id, &x, &y);
        if (xOffset == 0 && yOffset == 0) {
            xOffset = x;
            yOffset = y;
        }
        mainMap.points.push_back(new Point(id,  x -xOffset, yOffset -y));
        temp--;
    }

    if(numNodes == mainMap.points.size()) {
        cout << "Completed reading nodes\n";
    } else {
        cout << "Failed Reading nodes\n";
        exit(-1);
    }
    file.close();

    string edgeFile = "T09/" + cityName + "/T09_edges_" + cityName + ".txt";

    file.open(edgeFile);

    if (!file) {
        cout << "Failed to open edge file\n";
        exit(1);
    }
    getline(file, line);
    sscanf(line.c_str(), "%d", &temp);
    unsigned int numEdges = temp;
    id = 0;
    while (getline(file, line) && temp != 0) {
        sscanf(line.c_str(), "(%lf, %lf)", &x, &y);
        Point * source = findPoint(x);
        Point * dest = findPoint(y);
        Road * r1 = new Road(id,source, dest);
        Road * r2 = new Road(id,dest, source);
        source->addRoad(r1);
        dest->addRoad(r2);

        mainMap.roads.push_back(r1);
        temp--;
        id++;
    }

    if(numEdges == mainMap.roads.size()) {
        cout << "Completed reading edges\n";
    } else {
        cout << "Failed edges edges\n";
        exit(-1);
    }

    file.close();
}

void initMap() {
    gv = new GraphViewer(1000, 700, false);
    gv->createWindow(1000, 700);
    gv->defineVertexColor("black");
    gv->defineEdgeColor("black");
}

void updateColors(nodeEdge_t graph, int color) {
    string cor;
    switch (color)
    {
    case 1:
        cor = "GREEN";
        break;
    case 2:
        cor = "BLUE";
        break;
    case 3:
        cor = "PINK";
        break;
    case 4:
        cor = "LIGHT_GRAY";
        break;
    case 5:
        cor = "WHITE";
        break;
    case 6:
        cor = "MAGENTA";
        break;
    case 7:
        cor = "CYAN";
        break;
    case 8:
        cor = "GRAY";
        break;
    case 9:
        cor = "DARK_GRAY";
        break;
    default:
        cor = "BLACK";
        break;
    }

    for (long unsigned int i=1; i<graph.points.size(); i++) {
        if (graph.points[i]->getType()==DELIVERY)
            gv->setVertexColor(graph.points[i]->getID(),"YELLOW");
        else if (graph.points[i]->getType()==SOURCE)
            gv->setVertexColor(graph.points[i]->getID(),"ORANGE");
        else gv->setVertexColor(graph.points[i]->getID(),cor);
    }
    for (long unsigned int i=0; i<graph.roads.size(); i++) {
        gv->setEdgeColor(graph.roads[i]->getID(), cor);
    }

}

void displayMap(nodeEdge_t graph) {
    for(auto p : graph.points) {
        gv->addNode(p->getID(), p->getX(), p->getY());
    }
    for(auto e : graph.roads) {
        gv->addEdge(e->getID(),e->getSource()->getID(),e->getDest()->getID(), EdgeType::UNDIRECTED);
    }
    gv->rearrange();
}

void dijkstra(int sourceID, int destID) {
    cout << "Startin dijkstra\n";
    cout << sourceID<<"\t"<<destID<<endl;
    Point * source = findPoint(sourceID);
    Point * dest = findPoint(destID);
    double oldDistance;

    for(auto p : mainMap.points) {
        p->setDist(SIZE_MAX);
        p->setPath(nullptr);
        p->queueIndex = 0;
    }

    source->setDist(0);

    MutablePriorityQueue q;
    q.insert(source);

    while(!q.empty()) {

        source = q.extractMin();

        if(source->equals(*dest)) {
            break;
        }

        for(auto e : source->getRoads()) {
            oldDistance = e->getDest()->getDist();
            if(e->getDest()->getDist() > source->getDist() + e->getWeight()) {
                e->getDest()->setDist(source->getDist() + e->getWeight());
                e->getDest()->setPath(source);
                if(oldDistance == SIZE_MAX) {
                    q.insert(e->getDest());
                } else {
                    q.decreaseKey(e->getDest());
                }
            }
        }
    }
}

nodeEdge_t getPath(/*int sourceID, */int destID) {

    nodeEdge_t ret;
    int x=0;
    vector<Point *> path;
    vector<Road *> roads, temp;
    Point * dest = findPoint(destID);
    Point * source = dest->getPath();

    path.push_back(dest);

    while(source != nullptr) {
        temp=dest->getRoads();
        path.push_back(source);
        for (unsigned int i=0; i<temp.size(); i++)
            if (temp[i]->getDest()==source) {
                roads.push_back(temp[i]);
                x+=temp[i]->getWeight();
            }
        dest = source;
        source = source->getPath();
    }
    path.reserve(path.size());
    cout << "Ret path\n";
    ret.points = path;
    ret.roads = roads;
    ret.lenght = x;
    if (x==0) cout<<"Couldn't find path.\n";
    return ret;
}

void AdicionaEncomenda(int source, int delivery) {
    Package pacote;
    Point* Source;
    Point* Delivery;
    int ID = 0;

    pacote.setIdentifier(ID);
    cout << "ID of the source point?" << endl;
    //cin >> source;
    cout << "ID of the delivery point?" << endl;
    //cin >> delivery;
    Source = findPoint(source);
    Delivery = findPoint(delivery);
    if(Source != nullptr && Delivery != nullptr) {
        Source->setType(SOURCE);
        Delivery->setType(DELIVERY);
        pacote.setPickUpPoint(Source);
        pacote.setDeliveryPoint(Delivery);
        PackagesToDelivery[0].push_back(pacote);
        cout << "Your order has been added" << endl;
    }
    else cout << "Your order hasn't been added." << endl << "Please check is the points ID's are correct" << endl;

}

void AdicionaEncomenda() {
    Package pacote;
    Point* Source;
    Point* Delivery;
    int ID = 0;
    int source, delivery;
    cout << "Insert package ID: (no error verification, be careful) ";
    cin >> ID;
    pacote.setIdentifier(ID);
    cout << "ID of the source point?" << endl;
    cin >> source;
    cout << "ID of the delivery point?" << endl;
    cin >> delivery;
    Source = findPoint(source);
    Delivery = findPoint(delivery);
    if(Source != nullptr && Delivery != nullptr) {
        Source->setType(SOURCE);
        Delivery->setType(DELIVERY);
        pacote.setPickUpPoint(Source);
        pacote.setDeliveryPoint(Delivery);
        PackagesToDelivery.at(0).push_back(pacote); //MUDEI AQUI
        cout << "Your order has been added" << endl;
    }
    else cout << "Your order hasn't been added." << endl << "Please check if the points ID's are correct" << endl;
}

void distributePackages(int n) {
    vector<Package> vec;
    float angle, divangle=360/n;
    cout<<"Divangle:"<<divangle;
    for (int i=1; i<=n; i++) {
        PackagesToDelivery.push_back(vec);
    }
    cout << "SIZE" << PackagesToDelivery[0].size();
    for (long unsigned i=0; i<PackagesToDelivery[0].size(); i++) {
        angle = -180 / M_PI *atan2(PackagesToDelivery[0][i].getPickUpPoint()->getY()-centralPoint->getY(),PackagesToDelivery[0][i].getPickUpPoint()->getX()- centralPoint->getX());
        if (angle<0) angle+=360;
        cout<<"\t"<<angle;
        PackagesToDelivery[ceil(angle/divangle)].push_back(PackagesToDelivery[0][i]);
    }
}

std::vector<nodeEdge_t *> nearestNeighbour(std::vector<Package> packages, Point* supportPoint, int color) {

    cout << "Starting NN\n";
    std::vector<nodeEdge_t *> finalPaths; //vector a retornar

    std::vector<Point*> pointsToGo; //pontos a precorrer a cada iteração

    for(auto p : packages) { //PUP iniciais
        pointsToGo.push_back(p.getPickUpPoint());
        cout << "Added PUP "<<p.getPickUpPoint()->getID()<<"\t"<<p.getDeliveryPoint()->getID()<<endl;
    }

    std::vector<nodeEdge_t> dists;

    Point * currentPoint = supportPoint;
    while(pointsToGo.size() != 0) {
        //guarda em dists os caminhos todos calculados
        cout << "while\n";
        for(unsigned int i = 0; i < pointsToGo.size(); i++) {
            dijkstra(currentPoint->getID(), pointsToGo.at(i)->getID());
            dists.push_back(getPath(pointsToGo.at(i)->getID()));
        }

        auto bestPath = min_element(dists.begin(),dists.end(),compSL);//procura o menor caminho
        int temp =  bestPath - dists.begin();//index do menor caminho
        finalPaths.push_back(&(dists.at(temp)));//coloca o caminho no vetor a retornar
        updateColors(dists.at(temp),color);
        currentPoint = pointsToGo.at(temp);//atualiza a posição atual
        cout << "AAA: " << pointsToGo.at(temp)->getType() ;
        if(pointsToGo.at(temp)->getType() == SOURCE) { //Se o ponto era uma Source, coloca a delivery
            pointsToGo.at(temp) = packages.at(temp).getDeliveryPoint();
            cout << "switch\n";
        } else if(pointsToGo.at(temp)->getType() == DELIVERY) { //se era uma delivery retira do vetor
            pointsToGo.erase(pointsToGo.begin()+temp);
            cout << "remove\n";
        }
        cout << "Size "<< pointsToGo.size() << endl;
        dists.clear();//prepara proxima iteração
        cout << "Iter\n";
    }

    dijkstra(currentPoint->getID(), supportPoint->getID());
    auto retPath = getPath(supportPoint->getID());
    updateColors(retPath,color);
    finalPaths.push_back(&retPath);
    gv->setVertexColor(supportPoint->getID(),RED);

    return finalPaths;
}

void menuBase() {

    cout << endl;
    cout << " _______________________________________________________________________" << endl;
    cout << "|                         Chose one option                              |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|      1 - User mode                                                    |" << endl;
    cout << "|      2 - Company Mode                                                 |" << endl;
    cout << "|      3 - Exit                                                         |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|_______________________________________________________________________|" << endl;

    int opcao;


    cout << "Choose option: ";
    cin >> opcao;
    switch(opcao) {
    case 1: {
        menuUser();
        break;
    }
    case 2: {
        menuControler();
        break;
    }
    case 3:
        cout << "The program will end now!" << endl;
        exit(0);
    default:
        cout << "Sorry, not a valid choice. Choose again." << endl;
        menuBase();
        break;
    }

}

void menuControler() {
    cout << endl;
    cout << " _______________________________________________________________________" << endl;
    cout << "|                         Chose one option                              |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|      1- See the map                                                   |" << endl;
    cout << "|      2- See vehicle path to satisfy packages                          |" << endl;
    cout << "|      3- List all packages                                             |" << endl;
    cout << "|      4- See vehicle path to satisfy packages                          |" << endl;
    cout << "|      5- Exit                                                          |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|_______________________________________________________________________|" << endl;

    int opcao;
    cout << "Choose option: ";
    cin >> opcao;
    switch(opcao) {
    case 1: {
        cout << "The red points represent pickup points and the green ones represent delivery points." << endl;
        displayMap(mainMap);
        break;
    }
    case 2: {
        cout << "Define Central Point ID: ";
        int id;
        cin >> id;
        centralPoint = findPoint(id);
        break;
    }
    case 3: {
        cout << "Current packages to delivery:\n";

        for(auto p : PackagesToDelivery) {
            for(auto i : p)
                cout << "ID: " << i.getIdentifier() << "  Source: "<< i.getPickUpPoint()->getID()<<"  Destination: "<< i.getDeliveryPoint()->getID() <<endl;
        }

        cout << "\nReturning...";
        break;
    }
    case 4: {
        int trucksNo;
        cout << "How many trucks: ";
        cin >> trucksNo;
        distributePackages(trucksNo);

        for(unsigned int i = 1; i < PackagesToDelivery.size(); i++) {
            nearestNeighbour(PackagesToDelivery.at(i),centralPoint,i);
        }
        break;
    }
    case 5: {
        menuBase();
        break;
    }

    default:
        cout << "Wrong option. Try Again\n";
        menuControler();
        break;
    }

    menuBase();

}

void menuUser() {
    cout << endl;
    cout << " _______________________________________________________________________" << endl;
    cout << "|                         Chose one option                              |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|      1 - New Order                                                    |" << endl;
    cout << "|      2 - Remove Order                                                 |" << endl;
    cout << "|      3 - Exit                                                         |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|                                                                       |" << endl;
    cout << "|_______________________________________________________________________|" << endl;

    int opcao;
    cout << "Choose option: ";
    cin >> opcao;

    switch(opcao) {
    case 1: {
        AdicionaEncomenda();
        menuUser();
        break;
    }
    case 2: {
        int ID;
        cout << "Order ID to remove: ";
        cin >> ID;
        bool rem = false;
        for(auto p = PackagesToDelivery.begin(); p != PackagesToDelivery.end(); p++) {
            for(auto i = p->begin(); i != p->end(); i++)
                if(i->getIdentifier() == ID) {
                    p->erase(i);
                    rem = true;
                    break;
                }
        }
        if (rem) {
            cout << "Order removed\n";
            break;
        }
        else {
            cout << "Could not find ID\n";
            break;
        }

    }
    case 3: {
        menuBase();
        break;
    }
    default : {
        cout << "Wrong option. Try Again\n";
        menuUser();
        break;
    }
    }
    menuUser();
}

int main() {
    cout << "Write the name of the map you want.\n";
    cout << "Options:\nAveiro    Braga   Coimbra\n";
    cout << "Ermesinde Fafe    Gondomar\n";
    cout << "Lisboa*    Maia    Porto\n";
    cout << "Viseu     Portugal*\n";
    cout << "*Warning: large files\nMap Name: ";

    std::string mapName;
    cin >> mapName;

    initMap();
    readMap(mapName);

    if(mapName== "Aveiro")
        centralPoint = findPoint(1340357451);
    else if(mapName =="Braga")
        centralPoint = findPoint(403629927);
    else if(mapName == "Coimbra")
        centralPoint = findPoint(729338589);
    else if(mapName== "Ermesinde")
        centralPoint = findPoint(1113220224);
    else if(mapName == "Fafe")
        centralPoint = findPoint(26130605);
    else if(mapName== "Gondomar")
        centralPoint = findPoint(1165931709);
    else if(mapName == "Lisboa")
        centralPoint = findPoint(1206788150);
    else if(mapName == "Maia")
        centralPoint = findPoint(1179868708);
    else if(mapName == "Porto")
        centralPoint = findPoint(299608108);
    else if(mapName == "Viseu")
        centralPoint = findPoint(264357929);
    else if(mapName == "Portugal")
        centralPoint = findPoint(158862066);

    menuBase();
}
