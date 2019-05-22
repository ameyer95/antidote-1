all: main ;

main: main.o ASTNode.o PrettyPrinter.o
	g++ -o main main.o ASTNode.o PrettyPrinter.o

main.o: main.cpp ASTNode.h PrettyPrinter.h
	g++ -c main.cpp

ASTNode.o: ASTNode.cpp ASTNode.h
	g++ -c ASTNode.cpp

PrettyPrinter.o: PrettyPrinter.cpp PrettyPrinter.h ASTNode.h
	g++ -c PrettyPrinter.cpp

ConcreteSemantics.o: ConcreteSemantics.cpp ConcreteSemantics.h ASTNode.h DataSet.h
	g++ -c ConcreteSemantics.cpp

DataSet.o: DataSet.cpp DataSet.h
	g++ -c DataSet.cpp

clean:
	rm -f main *.o
