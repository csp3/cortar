#if __linux__
#define SO "LINUX"
#include <unistd.h>
#endif
#if _WIN32
#include <Windows.h>
#define SO "WINDOWS"
#endif

#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <dirent.h>

using namespace std;

const size_t MAXPARTES = 52;

////LLENAR ARCHIVO
void llenarArchivo(ifstream &arc, ofstream &arcnew, unsigned long int tamtotal, unsigned long int pini, unsigned long int pfin, int parte)
{
    char *byte = new char[1];
    string strpini = to_string(pini); 
    string strpfin = to_string(pfin); 
    char *car; 

    arcnew.seekp(0);
    for (size_t i = 0; i < strpini.length(); i++)
    {
        car = &strpini[i]; 
        arcnew.write(car, 1);
    }
    arcnew.write("-", 1); 
    for (size_t i = 0; i < strpfin.length(); i++)
    {
        car = &strpfin[i]; 
        arcnew.write(car, 1);
    }
    arcnew.write("-", 1); 
    while (pini < pfin)
    {
        //arc.seekg(pini); 
        arc.read(byte, 1);
        arcnew.write(byte, 1);
        pini++;         
    }
    // if(parte == 0)
        // pini = pini + 1;
    delete[] byte; 
}
////CORTAR
int cortarArchivo(ifstream &arc, char *argvA)
{
    int rep = 0;
    ofstream archivonew[MAXPARTES];
    unsigned long int maxt = 0;
    int np = 0, npaux = 0, res = 1;
    string a = "_cortado_";  
    string nom; 
    size_t i = 0;
    //lectura archivo origen 
    arc.open(argvA, ios::in | ios::binary);
    if (arc.fail())
    {
        cout << "Error al abrir archivo origen " << endl;
        rep = -1;
    }
    //archivo menores a 3.5 Gbytes
    arc.seekg(0, ios::end);
    maxt = arc.tellg();
    cout << "Size del archivo origen: " << maxt << " bytes" << endl;
    if (maxt > 3758096384) //3.5 Gbytes
    {
        cout << "Solo archivos menores a 3.50 Gbytes" << endl;
        rep = -1;
    }
    //numero de partes (50 como max.)
    cout << "Ingrese numero de partes (max. 50 partes): "; 
    cin >> np;
    while (cin.fail() || np > 50 || np < 2)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ingrese numero de partes (max. 50 partes): ";
        cin >> np;
    }
    //si se parte en np, se debe dividir en np - 1 (por el residuo) [se agrega un archivo al final pero es para unir]
    unsigned long int posini = 0, posfin = 0;
    arc.seekg(0); 
    if(maxt % np > 0)
        npaux = np - 1; 
    else 
        npaux = np;
    //cortando
    for (i = 0; i < np; i++)
    {
        posini = posfin;
        posfin = posini + (maxt / npaux);
        if(i == npaux && np != npaux)
            posfin = posini + maxt % npaux;
        //archivo nuevo 
        nom = argvA + a + to_string(i+1);
        archivonew[i].open(nom, ios::out | ios::trunc | ios::binary); 
        if (archivonew[i].fail())
        {
            cout << "Error al crear archivo: " << nom << endl;
            rep = -1;
        }
        cout << nom << endl;
        llenarArchivo(arc, archivonew[i], maxt, posini, posfin, i); 
        archivonew[i].close();
    }
    
    return rep;
}
////PTO.INICIO - PTO.FINAL
void cursorInicioFin(ifstream &arc, unsigned long int retorna[])
{
    char byte[1];
    int inicio = 0;
    size_t j = 0;
    char pipf[100];
    while (j < 3)
    {
        arc.read(byte, 1); 
        if(byte[0] == '-') 
            j++;
        pipf[inicio] = byte[0];
        inicio++;
        if(j==2) 
            break;
    } 
    int tamanio = inicio;  // tamaño de cadena que contiene:  "incio-final-"
    string pipfstr = string(pipf, tamanio);  
    inicio = pipfstr.find("-");
    //cursor incial 
    retorna[0] = stoi(pipfstr.substr(0, inicio)); 
    //cursor final 
    retorna[1] = stoi(pipfstr.substr(inicio + 1, tamanio - 1)); 
    //tamanio cadena
    retorna[2] = tamanio;
}
////ENLAZAR
void enlazar(ifstream &arc, ofstream &arcfinal, unsigned long int pi, unsigned long int pf, unsigned long int tam)
{
    char *byte = new char[1];
    arcfinal.seekp(pi); 
    arc.seekg(tam);
    while (pi < pf)
    {
        arc.read(byte, 1);
        arcfinal.write(byte, 1);
        pi++;
    }
    delete[] byte;
}
////PEGAR
int pegarArchivo(char *archivo1)
{
    int rep = 0;
    int numeroarc = 0;
    ifstream arc;
    ofstream arcFinal;
    string file, filebus, nomarchivos[MAXPARTES], nomArc1, nomArc; 
    struct dirent *d;
    DIR *directorioActual;
    unsigned long int ret[3];

    nomArc1 = string(archivo1);
    nomArc = nomArc1.substr(0, nomArc1.find("_cortado_1")); 

    arcFinal.open(nomArc, ios::out | ios::trunc | ios::binary);
    if (arcFinal.fail())
    {
        cout << "Error al crear archivo final " << endl;
        rep = -1;
    }
    //
    directorioActual = opendir(".");
    if(directorioActual!=NULL)
    {
        for(d = readdir(directorioActual); d!=NULL; d = readdir(directorioActual))
        {
            file = d->d_name;
            filebus = file.substr(0, file.find("_cortado_"));
            if(filebus.length() != file.length())
            {
                nomarchivos[numeroarc] = file; 
                numeroarc = numeroarc + 1;
            }
        }
        //pegando 
        for (size_t i = 0; i < numeroarc; i++)
        { 
            //leyendo desde archvios-partes
            arc.open(nomarchivos[i], ios::in | ios::binary);
            if(arc.fail())
            {
                cout << "Error al abrir archivo inicio " << endl;
                rep = -1;
            }
            cursorInicioFin(arc, ret); 
            enlazar(arc, arcFinal, ret[0], ret[1], ret[2]);
            arc.close();
        }
        arcFinal.close();
        closedir(directorioActual);
    }
    else
        rep = -1; 

    return rep;
}

////MAIN
int main(int argc, char *argv[])
{
    cout << endl;
    int res = 0;
    ifstream archivo;
    string argmain = string(argv[1]);
    
    if (argc != 3 || (argmain.compare("--cortar") != 0 && argmain.compare("-c") != 0 && argmain.compare("--unir") != 0 && argmain.compare("-u") != 0))
    {
        cout << "Para cortar: main.exe -c [--cortar] nombreArchivo " << endl;
        cout << "Para  pegar: main.exe -u [--unir] nombreArchivo_cortado_1 " << endl;
        return 1;
    }

    if(argmain.compare("--cortar") == 0 || argmain.compare("-c") == 0)
    {
        res = cortarArchivo(archivo, argv[2]); 
        archivo.close();
        if(res == -1 )
            return 1;
        else 
            cout << "..archivos Cortados !! " << endl; 
    }

    if(argmain.compare("--unir") == 0 || argmain.compare("-u") == 0)
    {
        res = pegarArchivo(argv[2]);
        if(res == -1)
            return 1;
        else 
            cout << "..archivos Unidos !! " << endl; 
    }
    
    cout << endl;
    return 0;
}
