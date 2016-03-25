/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qadjgraph.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/11/06
**
*********************************************************************************************/

#ifndef __QADJGRAPH_H_
#define __QADJGRAPH_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

#define DOUBLE_COST_MAX 99999999.99

// 邻接表结点类
template <typename Type>
struct Token {
	// 结点数据
	Type data;
	// 权值
	double cost;
	// 构造函数
	Token(const double weight=1.0): cost(weight) {}
	// 构造函数
	Token(const Type& d, const double weight=1.0): data(d), cost(weight) {}
};

// 邻接表的边
template <typename Type>
struct Edge {
	// 目标顶点
	int32_t dest;
	// 边上存储的数据结点
	Token<Type> token;
	// 下一条边
	Edge* link;
	// 构造函数
	Edge() {}
	// 构造函数
	Edge(const int32_t inVertex, const Token<Type>& inToken): dest(inVertex), token(inToken), link(NULL) {}
};

// 邻接表的顶点
template <typename Type>
struct Vertex {
	// 顶点编号
	int32_t vid;
	// 顶点所关联的边
	Edge<Type>* adj;
};

// 图的邻接表实现类
template <typename Type>
class QAdjGraph {
	public:
		// @函数名: 邻接表构造函数
		explicit QAdjGraph(const int32_t vertexNum) :
			numVertices(vertexNum), 
			numEdges(0)
		{
			nodeTable=q_new_array< Vertex<Type> >(vertexNum);
			Q_ASSERT(nodeTable!=NULL, "QAdjGraph: nodeTable is null, bad allocate!");
			for(int32_t i=0; i<vertexNum; i++) {
				nodeTable[i].vid=i;
				nodeTable[i].adj=NULL;
			}
		}

		// @函数名: 邻接表析构函数
		virtual ~QAdjGraph()
		{
			Edge<Type>* p=NULL;
			for(int32_t i=0; i<numVertices; i++) {
				p=nodeTable[i].adj;
				while(p) {
					nodeTable[i].adj=p->link;
					q_delete< Edge<Type> >(p);
					p=nodeTable[i].adj;
				}
			}
			q_delete_array< Vertex<Type> >(nodeTable);
		}

		// @函数名: 获取图中顶点的数据
		int32_t getVertexValue(const int32_t vertex) const
		{return (vertex>=0&&vertex<numVertices)?nodeTable[vertex].vid:-1;}

		// @函数名: 获取某条边上的数据
		bool getToken(const int32_t vertex1, const int32_t vertex2, Token<Type>& inToken) const
		{
			if(vertex1>=0&&vertex1<numVertices && vertex2>=0&&vertex2<numVertices) {
				Edge<Type> *p=nodeTable[vertex1].adj;
				while(p&&p->dest!=vertex2)
					p=p->link;
				if(p) {
					inToken=p->token;
					return true;
				}
			}
			return false;
		}

		// @函数名: 插入边
		bool insertEdge(const int32_t vertex1, const int32_t vertex2, const Token<Type>& inToken)
		{
			if(vertex1>=0&&vertex1<numVertices && vertex2>=0&&vertex2<numVertices) {
				Edge<Type> *p=nodeTable[vertex1].adj;
				while(p&&p->dest!=vertex2) p=p->link;	// Seek the adjacent vertex v2
				if(p) return false;			// Found the edge

				p=q_new< Edge<Type> >();		// Otherwise create an edge
				p->dest=vertex2;
				p->token=inToken;
				p->link=nodeTable[vertex1].adj;		// Link v1 edge list
				nodeTable[vertex1].adj=p;

				numEdges++;				// Total edges add 1
				return true;
			}
			return false;
		}

		// @函数名: 删除边
		bool removeEdge(const int32_t vertex1, const int32_t vertex2)
		{
			if(vertex1>=0&&vertex1<numVertices && vertex2>=0&&vertex2<numVertices) {
				Edge<Type> *p=nodeTable[vertex1].adj, *q=NULL, *s=p;
				while(p&&p->dest!=vertex2) {
					q=p;
					p=p->link;
				}
				if(p) {
					if(p==s) nodeTable[vertex1].adj=p->link;
					else q->link=p->link;
					q_delete< Edge<Type> >(p);
					numEdges--;
					return true;
				}
			}
			return false;
		}

		// @函数名: 获取某个顶点的第一个邻接顶点
		int32_t getFirstNeighbor(const int32_t vertex) const
		{
			if(vertex>=0&&vertex<numVertices) {
				Edge<Type> *p=nodeTable[vertex].adj;
				if(p!=NULL) return p->dest;
			}
			return -1;
		}

		// @函数名: 获取顶点数量
		int32_t getVertexNumber() const
		{return this->numVertices;}

		// @函数名: 获取边的数量
		int32_t getEdgeNumber() const
		{return this->numEdges;}

		// @函数名: 获取某个顶点所有边上的权值
		void getVertexEdgeCost(const int32_t vertex, std::vector<double>& distanceArray)
		{
			Edge<Type> *p=nodeTable[vertex].adj;
			while(NULL!=p) {
				distanceArray[p->dest]=p->token.cost;
				p=p->link;
			}
		}

		// @函数名: Dijkstra计算最短路径
		std::vector<int32_t> Dijkstra(const int32_t sourceVertex, const int32_t destVertex)
		{
			int32_t vertexNum=getVertexNumber();

			std::vector<bool> boolArray;
			boolArray.assign(vertexNum, 0);
			boolArray[sourceVertex]=1;

			std::vector<double> distanceArray;
			distanceArray.assign(vertexNum, DOUBLE_COST_MAX);
			distanceArray[sourceVertex]=0.0;

			std::vector<int32_t> prevVertexArray;
			prevVertexArray.assign(vertexNum, sourceVertex);

			std::vector<int32_t> shortestPath;

			getVertexEdgeCost(sourceVertex, distanceArray);

			while(1) {
				double minimumCost=DOUBLE_COST_MAX;
				int32_t vertexFrom=sourceVertex;
				int32_t vertexTo=-1;

				for(int32_t i=0; i<vertexNum; i++) {
					if(!boolArray[i]&&minimumCost>distanceArray[i]) {
						minimumCost=distanceArray[i];
						vertexFrom=i;
					}
				}
				if(minimumCost==DOUBLE_COST_MAX) break;
				boolArray[vertexFrom]=true;

				Edge<Type>* p=nodeTable[vertexFrom].adj;
				while(NULL!=p) {
					double edgeCost=p->token.cost;
					vertexTo=p->dest;
					if(!boolArray[vertexTo]&&distanceArray[vertexTo]>edgeCost+distanceArray[vertexFrom]) {
						distanceArray[vertexTo]=edgeCost+distanceArray[vertexFrom];
						prevVertexArray[vertexTo]=vertexFrom;
					}
					p=p->link;
				}
			}

#if 0
			// 输出所有顶点的最短路径
			std::cout<<"Shotest path from i to......"<<std::endl;
			for(int32_t i=0; i<vertexNum; i++) {
				if(distanceArray[i]!=DOUBLE_COST_MAX) {
					std::cout<<getVertexValue(sourceVertex)<<"->"<<getVertexValue(i)<<": ";
					DijkstraPrint(sourceVertex, i, prevVertexArray);
					std::cout<<" "<<vecDistanceArray[i];
					std::cout<<std::endl;
				}
			}
#endif

			DijkstraPrint(sourceVertex, destVertex, prevVertexArray, shortestPath);
			return shortestPath;
		}

		// @函数名: 邻接表的输出
		void printGraph()
		{
			int32_t vertexNum=getVertexNumber();
			printf("The adjacent graph has [%d] vertices:\n", vertexNum);
			for(int32_t i=0; i<vertexNum; i++) {
				printf("[%04d]: ", getVertexValue(i));
				Edge<Type> *p=nodeTable[i].adj;
				while(NULL!=p) {
					printf("(%d, %d, %0.2f) ", getVertexValue(i), getVertexValue(p->dest), p->token.cost);
					p=p->link;
				}
				printf("\n");
			}
		}

	private:
		void DijkstraPrint(int32_t sourceVertex, int32_t destVertex, std::vector<int32_t>& prevVertexArray, std::vector<int32_t>& shortestPath)
		{
			if(destVertex!=sourceVertex)
				DijkstraPrint(sourceVertex, prevVertexArray[destVertex], prevVertexArray, shortestPath);
			shortestPath.push_back(getVertexValue(destVertex));
		}

	protected:
		// 邻接表的顶点表
		Vertex<Type>* nodeTable;
		// 顶点数量
		int32_t numVertices;
		// 边数量
		int32_t numEdges;
};

Q_END_NAMESPACE

#endif // __QADJGRAPH_
