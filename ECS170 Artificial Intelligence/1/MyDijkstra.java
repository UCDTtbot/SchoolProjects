
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;
import java.util.PriorityQueue;
import java.util.Collections;
import java.util.Iterator;

/// Dijkstra AI to find the optimal path.
/**
 * Implementation of the Dijkstras algorithm provided in the book.
 * 
 * @author Tyler Welsh
 */
 public class MyDijkstra implements AIModule
{
    /// Creates the path to the goal.
    public List<Point> createPath(final TerrainMap map)
    {
        // Holds the resulting path - OUR FINAL ANSWER
        final ArrayList<Point> path = new ArrayList<Point>();
        // Keep track of where we are and add the start point.	
		final PriorityQueue<PathNode> frontier = new PriorityQueue<PathNode>();
		final ArrayList<Point> explored = new ArrayList<Point>();
		PathNode node = new PathNode(map.getStartPoint(), 0, null);
		boolean goal = false;
		frontier.add(node);
		//frontier.indexOf(node);
		explored.add(node.getPoint());
		int count = 0;
		while(!goal)
		{
			if(frontier.size() == 0)
			{
				System.out.println("Frontier failed, returned uncomplete path");
				return path;
			}
			node = frontier.poll();
			if((node.getPoint().x == map.getEndPoint().x) && node.getPoint().y == map.getEndPoint().y)
			{
				System.out.println("Found!");
				goal = true;
				while(node != null)
				{
					path.add(node.getPoint());
					node = node.getParent();
				}
				Collections.reverse(path);
				System.out.println("Start Point of Path: " + path.get(0) + " and ends at: " + path.get(path.size() - 1));
				//return path;
			}
			//return path;
			if(!goal)
			{
				explored.add(node.getPoint());
				final Point[] children = map.getNeighbors(node.getPoint());
				//System.out.println("Current Position: (" + node.getPoint().x + ", " + node.getPoint().y + ")" + " Cost: " + node.getNodeCost());
				for(int i = 0; i < children.length; i++)
				{
					PathNode newNode = new PathNode(children[i], map.getCost(node.getPoint(), children[i]), node);
					if(!(frontier.contains(newNode)) && !(explored.contains(newNode.getPoint())))
					{
						if(!(frontier.contains(newNode)))
							frontier.add(newNode);
						else if(frontier.contains(newNode))
						{
							Iterator<PathNode> itr = frontier.iterator();
							boolean exit = false;
							while(itr.hasNext() && !exit)
							{
								PathNode n = itr.next();
								if(n.equals(newNode) && n.getNodeCost() > newNode.getNodeCost())
								{
									frontier.remove(n);
									frontier.add(newNode);
								}
							}
						}
						//else if(frontier.contains(newNode) && 
						//System.out.println("Added Child: " + i + " Position: (" + newNode.getPoint().x + ", " + newNode.getPoint().y + ")" + " Cost: " + newNode.getNodeCost());
					}
				}
			}
			//System.out.println("Frontier now has: " + frontier.size() + " nodes");
			//System.out.println("Head is node: (" + frontier.peek().getPoint().x + ", " + frontier.peek().getPoint().y + ")" + " Cost: " + frontier.peek().getNodeCost()); 
			//if(count == 50000)
			//	goal = true;
			//count++;
		}
		//PathNode has getNodeCost() and getPoint()
		//System.out.println("Constructed Node with Coords and Cost: " + node.getPoint().x + " " 
		//																				+ node.getPoint().y + " " + node.getNodeCost()) ;
		//final PathNode testNode = new PathNode(new Point(0, 0), 9);
		//final PathNode testNode2 = new PathNode(new Point(0,0), 10);
		//int result = testNode.compareTo(node);
		//System.out.println("Should be -1, got: " + result);

		//Useful functions: map.getEndPoint() gets the end point of the map, both .x and .y
		// PATH is our final answer
		// FRONTIER is our currently working path
		//	grab a node from frontier with smallest cost
		
		
		

		
		/* path.clear();
		// STUPID AI FOR TESTING 
		 path.add(new Point(CurrentPoint));	// adds the start point
		 // Keep moving horizontally until we match the target.
        while(map.getEndPoint().x != CurrentPoint.x)
        {
            if(map.getEndPoint().x > CurrentPoint.x)
                ++CurrentPoint.x;
            else
                --CurrentPoint.x;
            path.add(new Point(CurrentPoint));
        }

        // Keep moving vertically until we match the target.
        while(map.getEndPoint().y != CurrentPoint.y)
        {
            if(map.getEndPoint().y > CurrentPoint.y)
                ++CurrentPoint.y;
            else
                --CurrentPoint.y;
            path.add(new Point(CurrentPoint));
        }

        // We're done!  Hand it back.
        return path; */
		return path;	
    }
	/**
	* function CHILD-NODE(problem, parent, action) returns a point
	*	return a point with
	*		STATE = problem.RESULT(parent.STATE, action
	*		PARENT = parent, ACTION = action
	*		PATH-COST = parent.PATH-COST + problem.STEP-COST(parent.STATE, action)
	*/
	
	/**
	* function UNIFORM-COST-SEARCH(problem) returns a solution, or failure
	*	node <- a node with STATE = problem.INITIAL.STATE, PATH-COST = 0
	*	frontier <- a priority queue ordered by PATH-COST, with node as the only element
	*	explored <- empty
	*	loop do
	*		if EMPTY?(frontier) return failure
	*		node <- POP(frontier) chooses the lowest cost node in frontier 
	*		if problem.GOAL-TEST(node.STATE) then return SOLUTION(node)
	*		add node.STATE to explored
	*		for each action in problem.ACTIONS(node.STATE) do
	*		child <- CHILD-NODE(problem, node, action)
	*		if child.STATE is not in explored or frontier then
	*			frontier <- INSERT(child, frontier)
	*		else if child.STATE is in frontier with higher PATH-COST then
				replace that frontier node with child
	*/
	public class PathNode implements Comparable<PathNode>
	{
		private Point nodePoint;
		private PathNode parent;
		private double cost;
		
		public PathNode(Point pnt, double pntCost, PathNode par)
		{
			nodePoint = pnt;
			cost = pntCost;
			parent = par;
		}
		
		@Override
		public int compareTo(PathNode o)
		{
			double delta = this.cost - o.getNodeCost();
			if(delta > 0) return 1;
			else if(delta < 0) return -1;
			else return 0;
			
		}
		
		@Override
		public boolean equals(Object o)
		{
			if(!(o instanceof PathNode))
			{
				return false;
			}
			PathNode other = (PathNode) o;
			
			return (this.getPoint().equals(other.getPoint()));
		}
		
		public double getNodeCost()
		{
			return cost;
		}
		
		public Point getPoint()
		{
			return nodePoint;
		}
		
		public PathNode getParent()
		{
			return parent;
		}
	}
	

}

