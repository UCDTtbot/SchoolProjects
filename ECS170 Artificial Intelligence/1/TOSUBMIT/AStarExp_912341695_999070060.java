
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Set;

/// Dijkstra AI to find the optimal path.
/**
 * Implementation of the Dijkstras algorithm with help from Lars Vogel following his
 * Dijkstra Tutorial at vogella.com/tutorials/JavaAlgorithmsDijkstra/article.html
 * 
 * @author Tyler Welsh and Alice Chen, with help from Lars Vogel from vogella.com
 */
 public class AStarExp_912341695_999070060 implements AIModule
{
	int check = 0;
		//Set for our Explored List
	private Set<Point> explored;
		//Set for our Frontier List. Using Set instead of Queue because Queue gave me issues
	private Set<Point> frontier;
		//HashMap to map node to parent
	private Map<Point, Point> parentsMap;
		//HashMap to map nodes to costs
	private Map<Point, Double> costMap;
	private Map<Point, Double> gCosts;
	private boolean done;
	
    /// Creates the path to the goal.
    public List<Point> createPath(final TerrainMap map)
    {
        // Holds the resulting path - OUR FINAL ANSWER
        final ArrayList<Point> path = new ArrayList<Point>();
																					//PathNode(Point pnt, PathNode parent, TerrainMap map)
			//Set for our Explored List
		explored = new HashSet<Point>();
			//Set for our Frontier List. Using Set instead of Queue because Queue gave me issues
		frontier = new HashSet<Point>();
			//HashMap to map node to parent
		parentsMap = new HashMap<Point, Point>();
			//HashMap to map nodes to costs
		costMap = new HashMap<Point, Double>();
		gCosts = new HashMap<Point, Double>();
		done = false;
			//Init the start point, map its costs, and add to frontier
		Point startPoint = map.getStartPoint();
		costMap.put(startPoint, 0.0);
		gCosts.put(startPoint, 0.0);
		frontier.add(startPoint);
			//While the frontier is not empty
		System.out.println("Starting exploration");
		while(frontier.size() > 0 && !done)
		{
				//Find the smallest node in the frontier. This is kind of a psuedo priority queue.
				//I was having a god awful time traversing a priority queue, so this is the alternative
				//Instead of just popping off the smallest cost from the front, we just traverse the whole damn
				//frontier to find the smallest one. If this becomes too slow, will try to reimplement of prio queue
			Point minPoint = null;
			for(Point pnt : frontier)
			{
				if(minPoint == null)
					minPoint = pnt;
				else if(getPntCost(pnt) < getPntCost(minPoint))
					minPoint = pnt;
				//if(check!= 0)
				//	System.out.println(map.getCost(minPoint, parentsMap.get(minPoint)));
				//System.out.println("---------------------------------------");
			}
			if(minPoint.equals(map.getEndPoint()))
				done = true;
				//Add the smallest node to explored list
			explored.add(minPoint);
				//Remove the smallest node from the frontier
			frontier.remove(minPoint);
				//Find all adj nodes and add their values to the costs map and parents map
			expandPnts(minPoint, map);
			
			//while(check == 3000) {}
			//check++;
		}

			//Now we need to construct that path after all nodes are expanded
		Point curPnt = map.getEndPoint();
			//if for some reason the end point doesn't have a parent, return failure
			//mainly just error checking here
		if(parentsMap.get(curPnt) == null)
		{
			System.out.println("Failure to find");
			return null;
		}
			//add the end point to the path
		path.add(curPnt);
			//while the current node still has a parent, looop
			//This works because the start node was given a parent of NULL
		System.out.println("Explored all, constructing path");
		while(parentsMap.get(curPnt) != null)
		{
				//Get the current nodes parent
			curPnt = parentsMap.get(curPnt);
				//and add it to the path
			path.add(curPnt);
		}
			//The above loop gives us the path from end to start
			//we need start to end, so reverse
		Collections.reverse(path);
		
		return path;	
    }
	
		private void expandPnts(Point curPnt, TerrainMap map)
	{
			//get all adj nodes to the current nodes using getNeighbors from TerrainMap
		final Point[] adjPoints = map.getNeighbors(curPnt);
			//Run through the array and expand each point
		Point adjPnt;
		for(int i = 0; i < adjPoints.length; i++)
		{
			adjPnt = adjPoints[i];
				//This is the whole "if the current point already exists in the frontier, but with a higher cost
				//replace it with the new point of lower cost" thing
				//because we could not "find and remove" something from the priority queue, instead we're using
				//a hash set and using traversal to find the smallest
			//if(getPntCost(adjPnt) > getPntCost(curPnt) + map.getCost(curPnt, adjPnt))
			
			//double gn = getPntCost(curPnt) + map.getCost(curPnt, adjPnt);
			double gn = getGCost(curPnt) + map.getCost(curPnt, adjPnt);
			
			//double hn = getHeurDiv(map, curPnt, adjPnt);
			double hn = getHeuristic(map, curPnt, adjPnt);
			
			if((getPntCost(adjPnt) > gn +  hn && !explored.contains(adjPnt))) 
			{
					//add the adjNode (child) and its total cost to the costs map
				costMap.put(adjPnt, gn + hn );
				gCosts.put(adjPnt, gn );
					//add the adjNode to the parents map with its parent curNode
				parentsMap.put(adjPnt, curPnt);
					//add the new adj nodes to the frontier
				frontier.add(adjPnt);
			}
		}
	}
	
	private Double getHeuristic(final TerrainMap map, final Point pt1, final Point pt2)
	{
		double dx = Math.abs(map.getEndPoint().x - pt1.x);
		double dy = Math.abs(map.getEndPoint().y - pt1.y);
		double pyth = Math.sqrt(dx * dx + dy *dy) ;
		double hC = map.getTile(pt1);
		double hE = map.getTile(map.getEndPoint());
		double hS = map.getTile(map.getStartPoint());
		double hMin;
		double D;
		double hn;
		
		final Point[] adjPoints = map.getNeighbors(map.getEndPoint());
		Point adjPnt;
		Point minPnt = adjPoints[0];
		for(int i = 0; i < adjPoints.length; i++)
		{
			adjPnt = adjPoints[i];
			if(map.getTile(adjPnt) < map.getTile(minPnt))
				minPnt = adjPnt;
		}
		
		D = (Math.exp(map.getTile(map.getEndPoint()) - map.getTile(map.getEndPoint()) - map.getTile(minPnt)));

		hn = D * pyth;
		
		return hn;
	}

	private int factorial(int n)
	{
		if(n==0) return 1;
		return n * factorial(n-1);
	}
	
	private Double getPntCost(Point pnt)
	{
			//Find the nodes relevant cost in the costs map and return it
		Double cost = costMap.get(pnt);
			//if the point doesn't yet exist, return some super high number to signify this fact
		if(cost == null)
		{
			return Double.MAX_VALUE;
		}
		else
		{
				//return the cost
			return cost;
		}
	}
	
	private Double getGCost(Point pnt)
	{
		Double cost = gCosts.get(pnt);
		if(cost == null)
			return Double.MAX_VALUE;
		else
			return cost;
	}
}
