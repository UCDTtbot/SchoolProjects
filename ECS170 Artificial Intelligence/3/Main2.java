import java.io.*;
//import java.util.Scanner;
//import java.util.ArrayList;
//import java.util.List;
//import java.util.Random;
//import java.util.HashMap;
import java.util.*;
import java.lang.Math;

public class Main2
{
    public static File[] maleFiles;
    public static File[] femaleFiles;
    public static File[] testFiles;
    public static boolean test = false;
    public static boolean train = false;
    public static double[] male = {0.2, 0.8};
    public static double[] female = {0.8, 0.2};
    public static double[] testOut = {0.0, 0.0};
    public static double[] out = new double[2];
    public static final int WIDTH = 128;
    public static final int HEIGHT = 120;
    public static boolean terminate = false;
    public static Map<Key, Double> weights = new HashMap<Key,Double>();
    public static Map<Key, Double> hiddenWeights = new HashMap<Key,Double>();

    public static final double learnRate = 0.05;
    public static final int rep = 25;
    public static double[] finalAns = {0, 0};
    public static final int hiddenUnits = 2;

    
    /// Program startup function.
    public static void main(String[] args)
    {
        maleFiles = openFiles("/Male");
        femaleFiles = openFiles("/Female");
        testFiles = openFiles("/Test");

        parseArgs(args);

        if(train)
        {
            System.out.println("Train");
            initWeights(hiddenUnits, testOut.length);
            backPropagation(maleFiles, learnRate, male, hiddenUnits, rep);
            backPropagation(femaleFiles, learnRate, female, hiddenUnits, rep);
        }
        else if(test)
        {
            System.out.println("Test");
            //backPropagation(testFiles, testOut, learnRate, rep);
        }
        else
            System.out.println("No command line argument, exiting.");

    }

    //Backpropagation algorithm
    /**
     * Creates a new game board of the specified width and height.  The starting player
     * is player 1 and the board is initially empty and with no undo/redo history.
     *
     * @param input The vector of image files.
     * @param output The mapping for desired output male/female.
     * @param
     */
    public static void backPropagation(File[] input, double rate, double[] output, int num_units, int nreps)
    {
        /*
            Each training example is a pair of the form <x, t> where x is the vector of network input values
            and t is the vector of the target network output values

            n is the learning rate (eg., .05). n_in is the number of network inputs, n_hidden the number of units
            in the hidden layer, and n_out the number of output units.

            The input from unit i into unit j is denoted x_ji, and the weight from unit i to unit j is denoted w_ji

            * Create a feed-forward network with n_in inputs, n_hidden hidden units, and n_out output units
            * Initialize all network weights to small random numbers (eg., between -0.5 and 0.5)
            * Until the termination condition is met, Do:
                * For each <x, t> in training_examples, Do
                    Propagate the input forward through the network:
                    1. Input the instance x to the network and compute the output o_u of every unit u in the network
                    Propagate the errors backward through the network:
                    2. For each network output unit k, calculate its error term s_k
                        s_k = o_k(1 - o_k)(t_k - o_k)
                    3. For each hidden unit h, calculate its error term s_h
                        s_h = o_h(1 - o_h) * sum_{for k in output}(w_kh * s_k)
                    4. Update each network weight w_ji
                        w_ji = w_ji + {delta}w_ji
                    where
                        {delta}w_ji = n* s_j * x_ji
        */ 

        // File Reading
        Scanner scanner;
        //Map<Key, Double> blockOne = new HashMap<Key, Double>();
        //Map<Key, Double> blockTwo = new HashMap<Key, Double>();
        Map<Key, Double> networkIn = new HashMap<Key, Double>();
        int wid = 0;
        int heig = HEIGHT;
        int curRep = 0;
                //The below code steps through all files from a given folder.
        //This is the FOR EACH INPUT step
        while(!terminate)
        {
            curRep++;
            for(File file : input)
            {
                try
                {
                    scanner = new Scanner(file);
                    wid = 0;
                    heig = 0;
                    while(scanner.hasNext())
                    {
                        // if(scanner.hasNextInt() && heig < 100 && heig >= 50 && wid >= 20 && wid <= 108)
                        // {
                        //     blockOne.put(new Key(wid, heig), (double)scanner.nextInt());
                        //     //System.out.println("Added: " + wid + ", " + heig + " to BLOCK ONE with value: " + blockOne.get(blockOne.size()-1));
                        //     wid++;
                        // }
                        // else if(scanner.hasNextInt() && heig < 50  && heig >= 0 && wid >= 20 && wid <= 108)
                        // {
                        //     blockTwo.put(new Key(wid, heig), (double)scanner.nextInt());
                        //     //System.out.println("Added: " + wid + ", " + heig + " to BLOCK TWO with value: " + blockTwo.get(blockTwo.size()-1));
                        //     wid++;
                        // }
                        if(scanner.hasNextInt())
                        {
                            networkIn.put(new Key(wid, heig), (double)scanner.nextInt());
                            wid++;
                        }
                        else
                        {
                            scanner.next();
                            wid++;
                        }

                        if(wid == WIDTH - 1)
                        {
                            heig++;
                            wid = 0;
                        }
                    }
                }
                catch(FileNotFoundException fnfe)
                {
                    System.out.println(fnfe.getMessage());
                }
                
                double[] net_j = new double[num_units];
                double[] out_j = new double[num_units];
                for(int h = 0; h < num_units; h++)
                {
                    net_j[h] = hiddenOut(networkIn, h);
                    //System.out.println(net_j[h]);
                    out_j[h] = 1.0 / (1.0 + Math.exp(1.0 - net_j[h]));
                }

                double[] net_k = new double[output.length];
                double[] out_k = new double[output.length];
                for(int k = 0; k < output.length; k++)
                {
                    net_k[k] = finalOut(out_j, k);
                    out_k[k] = 1.0 / (1.0 + Math.exp(1.0 - net_k[k]));
                }

                double error_k[] = new double[output.length];
                for(int k = 0; k < output.length; k++)
                {
                    error_k[k] = out_k[k] * ( 1.0 - out_k[k] ) * ( output[k] - out_k[k] );
                }

                double error_j[] = new double[num_units];
                for(int j = 0; j < num_units; j++)
                {
                    double sum = 0;
                    for(int k = 0; k < output.length; k++)
                    {
                        sum += error_k[k] * hiddenWeights.get(new Key(j, k));
                    }
                    error_j[j] = out_j[j] * ( 1.0 - out[j] ) * sum;
                }

                for(int i = 0; i < 15360; i++)
                {
                    double newValue = 0;
                    for(int j = 0; j < num_units; j++)
                    {
                        newValue = rate * error_j[j] * networkIn.get(new Key(i / WIDTH, i % 120));
                        newValue += weights.get(new Key(i, j));
                        weights.put(new Key(i, j), newValue);
                    }
                }

                for(int j = 0; j < num_units; j++)
                {
                    double newValue = 0;
                    for(int k = 0; k < output.length; k++)
                    {
                        newValue = rate * error_k[k]  * out_j[j];
                        newValue += hiddenWeights.get(new Key(j, k));
                        hiddenWeights.put(new Key(j, k), newValue);
                    }
                }
                //Now we have the current image sectioned off in 2 blocks to feed to the output
                //Hidden one
                // for(Map.Entry<Key,Double> entry : blockOne.entrySet())
                // {
                //     Key key = entry.getKey();
                //     Double value = entry.getValue();
                //     net[0] += weights.get(key) * value;
                //     //net[1] += weights.get(key) * value;

                //     //System.out.println("Weight exists: " + weights.get(key));
                //     //System.out.println("Weight: " + weights[key.x * key.y] + ". Value: " + value + ". Net:" + net[0]);
                // }
                // //Hidden two
                // for(Map.Entry<Key,Double> entry : blockTwo.entrySet())
                // {
                //     Key key = entry.getKey();
                //     Double value = entry.getValue();
                //     //net[0] += weights.get(key) * value;
                //     net[1] += weights.get(key) * value;
                // }
                //System.out.println("Got: " + out_k[0] + " " + out_k[1] + " True: " + output[0] + " " + output[1]);
                //checkAns(out_k, output);
                System.out.println("Out: " + out_k[0] + " " + out_k[1] + " Answer: " + output[0] + " " + output[1]);

            }
            if(curRep == nreps)
                terminate = true;
        }

    }


    public static File[] openFiles(String path)
    {
        String filePath = new File("").getAbsolutePath();
        filePath = filePath.concat(path);
        File folder = new File(filePath);
        System.out.print(path + " exists: ");
        System.out.println(folder.exists());
        File[] listOfFiles = folder.listFiles();

        return listOfFiles;
    }

    public static double hiddenOut(Map<Key, Double> in, int curUnit)
    {
        double net = 0;
        int i = 0;
        for(Map.Entry<Key,Double> entry : in.entrySet())
        {
            Key key = entry.getKey();
            Double value = entry.getValue();
            net += weights.get(new Key(i, curUnit)) * value;
            i++;
        }
        return net;
    }

    public static double finalOut(double[] in, int k)
    {
        double net = 0;
        for(int j = 0; j < in.length; j++)
        {
            net += hiddenWeights.get(new Key(j, k)) * in[j];
        }
        return net;
    }

    public static void initWeights(int hidden, int output)
    {
        double start = 0.0001;
        double end = 0.001;
        Random random = new Random();
        Scanner scanner;
        int wid = 0;
        int heig = HEIGHT;
        
        for(int i = 0; i < 15360; i++)
        {
            for(int j = 0; j < hidden; j++)
            {
                weights.put(new Key(i, j), start+(end-start)*random.nextDouble());
            }
        }

        for(int j = 0; j < hidden; j++)
        {
            for(int k = 0; k < output; k++)
            {
                hiddenWeights.put(new Key(j,k), start+(end-start)*random.nextDouble());
            }
        }
        // for(Map.Entry<Key,Double> entry : weights.entrySet())
        // {
        //     Key key = entry.getKey();
        //     Double value = entry.getValue();
        //     System.out.println("Weight: " + key.x + " " + key.y + " V: " + value);
        // }
    }

    public static void checkAns(double[] ans, double[] trueAns)
    {
        if(ans[0] == trueAns[0] && ans[1] == trueAns[1])
        {
            System.out.println("Exact Match");
        }
        else if( (trueAns[0] - ans[0]) <= 0.1 && (trueAns[0] - ans[0]) >= -0.1 && (trueAns[1] - ans[1]) <= 0.1 && (trueAns[1] - ans[1]) >= -0.1)
        {
            System.out.println("Within 0.1");
        }
    }

    public static void parseArgs(String[] args)
    {

        try
        {
            int i = 0;
            while(i < args.length)
            {
                if(args[i].equalsIgnoreCase("-train"))
                    train = true;
                else if(args[i].equalsIgnoreCase("-test"))
                    test = true;
                i++;                
            }
        }
        catch(IllegalArgumentException ia)
        {
            System.err.println("Invalid Arguments: " + ia.getMessage());
            System.exit(4);
        }
        catch(Exception e)
        {
            System.err.println("Unknown Error");
            System.exit(5);
        }

    }

    public static class Key
    {
        public final int x;
        public final int y;

        public Key(int x, int y)
        {
            this.x = x;
            this.y = y;
        }

        @Override
        public boolean equals(Object o)
        {
            if(this == o) return true;
            if(!(o instanceof Key)) return false;
            Key key = (Key) o;
            if(x == key.x && y == key.y)
                return true;
            else
                return false;
        }

        @Override
        public int hashCode()
        {
            int result = x;
            result = 31*result + y;
            return result;
        }

    }
}

