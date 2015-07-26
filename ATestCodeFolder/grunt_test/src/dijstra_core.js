
/* 计算有向无环图中，指定两点之间的最短路径 */
function ComputeShortPath(map, startName, endName)
{
    //判断两个节点是否有前后继关系，第一个为前，后一个为后
    function IsSuccessor(nodeNameA, nodeNameB)
    {
        var page = pages[nodeNameA];
        var nextNodes = page["nextNode"];
        for ( var index in nextNodes )
        {
            var nextNode = nextNodes[index];
            if ( nextNode == nodeNameB )
            {
                return true;
            }
        }
        
        return false;
    }

    /* 生成权重矩阵 */
    function GenerateEdgeWeight(map)
    {
        document.write("---- Generate edgeWeightMatrix ----</br>")
        
        /* 记录图中各个点之间有向边的权重 */
        var edgeWeightMatrix = [];
    
        for ( var nodeNameA in map )
        {
            edgeWeightMatrix[nodeNameA] = [];
            
            for ( var nodeNameB in map )
            {
                var isSucc = IsSuccessor(nodeNameA, nodeNameB);
                var weight;
                if ( true == isSucc )
                {
                    /* 有前后继的关系，权重设置为1 */
                    weight = 1;
                }
                else
                {
                    /* 没有前后继的关系，权重设置为正无穷 */
                    weight = Number.POSITIVE_INFINITY;
                }
                edgeWeightMatrix[nodeNameA][nodeNameB] = weight;
                
                document.write("edgeWeightMatrix["+nodeNameA+"]["+nodeNameB+"]="+edgeWeightMatrix[nodeNameA][nodeNameB]+"</br>")
            }
        }
        
        return edgeWeightMatrix;
    }

    /* 产生Dijistra运算的数据结构 */
    function GenerateDijistraDataStruct(map, startName, edgeWeightMatrix)
    {
        var dijistraJSON = {
            /* 已经知道最短路径集合 */
            knownSet:[], 
            /* 未知最短路径集合 */
            unKnownSet:[], 
            /* 最短特殊路径长度 */
            distanceSP:[],
        };
        
        /* 初始化，出发点添加到已知集合，其他添加到未知集合 */
        for ( var nodeName in map )
        {
            if ( nodeName == startName )
            {
                dijistraJSON.knownSet.push(startName);
            }
            else
            {
                dijistraJSON.unKnownSet.push(nodeName);
            }
        }
        
        /* 初始化最短特殊路径 */
        document.write("---- Generate distanceSP with startName = "+startName+" ----</br>")
        for ( var index in dijistraJSON.unKnownSet )
        {
            var nodeName = dijistraJSON.unKnownSet[index];
            dijistraJSON.distanceSP[nodeName] = edgeWeightMatrix[startName][nodeName];
            document.write("distanceSP["+nodeName+"]="+dijistraJSON.distanceSP[nodeName]+"</br>");
        }
        
        return dijistraJSON;
    }

    /* 运行最短路计算逻辑 */
    function DoDijistraComputing(dijistraJSON, edgeWeightMatrix)
    {
        /* dijstra */
        var loopNum = dijistraJSON.unKnownSet.length;
        for ( var i=0; i<loopNum; i++ )
        {
            /* 找unKnownSet中最短特殊路径顶点 */
            var minDistName = dijistraJSON.unKnownSet[0]; 
            var minDistIndex = 0;
            var minDist = dijistraJSON.distanceSP[minDistName];
            for ( var index in dijistraJSON.unKnownSet )
            {
                var nodeName = dijistraJSON.unKnownSet[index];
                var dist = dijistraJSON.distanceSP[nodeName];
                if ( dist < minDist )
                {
                    minDist = dist;
                    minDistName = nodeName;
                    minDistIndex = index;
                }
            }
            
            /* 将最短特殊路径顶点，从unkownSet中删除，加入knownSet */
            dijistraJSON.unKnownSet.splice(minDistIndex, 1);
            dijistraJSON.knownSet.push(minDistName);
            
            /* 更新 unKnownSet 中元素的 distanceSP 值 */
            for ( var index in dijistraJSON.unKnownSet )
            {
                var nodeName = dijistraJSON.unKnownSet[index];
                var oldDist = dijistraJSON.distanceSP[nodeName];
                var weight = edgeWeightMatrix[minDistName][nodeName];
                var newDist = minDist + weight;
                if ( newDist < oldDist )
                {
                    dijistraJSON.distanceSP[nodeName] = newDist;
                }
            }
        }

        /* 输出看打印结果 */
        document.write("---- Complete distanceSP with startName = "+startName+" ----</br>")
        for ( var index in dijistraJSON.knownSet )
        {
            var nodeName = dijistraJSON.knownSet[index];
            document.write("distanceSP["+nodeName+"]="+dijistraJSON.distanceSP[nodeName]+"<br/>");
        }
        
        return dijistraJSON;
    }

    /* 获取最短路径 */
    function GetShortPath(dijistraJSON, endName)
    {
        return dijistraJSON.distanceSP[endName];
    }
    
    var edgeWeightMatrix = GenerateEdgeWeight(map);
    var dijistraJSON = GenerateDijistraDataStruct(map, startName, edgeWeightMatrix);
    dijistraJSON = DoDijistraComputing(dijistraJSON, edgeWeightMatrix);
    var shortPath = GetShortPath(dijistraJSON, endName);
    
    document.write("---- Complete shortpath between "+startName+" and "+endName+" = "+shortPath+"----</br>")
}
