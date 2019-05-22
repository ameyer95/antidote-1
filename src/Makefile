all: main ;

main: main.o ASTNode.o PrettyPrinter.o
	g++ -o main main.o ASTNode.o PrettyPrinter.o

main.o: main.cpp ASTNode.h
	g++ -c main.cpp

ASTNode.o: ASTNode.cpp ASTNode.h
	g++ -c ASTNode.cpp

PrettyPrinter.o: PrettyPrinter.cpp PrettyPrinter.h ASTNode.h
	g++ -c PrettyPrinter.cpp

clean:
	rm -f main main.o ASTNode.o PrettyPrinter.o
