import java.io.*;
//import java.util.Scanner;
//import java.util.ArrayList;
//import java.util.List;
//import java.util.Random;
//import java.util.HashMap;
import java.util.*;
import java.lang.Math;

public class Main3Split
{
    public static File[] maleFiles;
    public static File[] femaleFiles;
    public static File[] testFiles;
    public static boolean test = false;
    public static boolean train = false;
    public static boolean crossTest = false;
    public static double[] maleAns = {0.4, 0.6};
    public static double[] femaleAns = {0.6, 0.4};
    public static double[] testOut = {0.0, 0.0};
    public static double[] out = new double[2];
    public static final int WIDTH = 128;
    public static final int HEIGHT = 120;
    public static boolean terminate = false;
    public static Map<Key, Double> weights = new HashMap<Key,Double>();
    public static Map<Key, Double> hiddenWeights = new HashMap<Key,Double>();

    public static final double learnRate = 0.005;
    public static final int rep = 20;
    public static double[] finalAns = {0, 0};
    public static final int blocks = 3;
    public static final int hiddenUnits = 6;
    public static final Map<File, double[]> answerMap = new HashMap<File, double[]>();

	
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
            initWeights(hiddenUnits, blocks, testOut.length);
            File[] merged = mergeFiles(maleFiles, femaleFiles);
            backPropagation(merged, blocks, learnRate, answerMap, hiddenUnits, rep);
            //backPropagation(femaleFiles, learnRate, female, hiddenUnits, rep);
            //test(testFiles, hiddenUnits);
            saveWeights();
        }
        else if(crossTest)
        {
            System.out.println("Cross Test");
            initWeights(hiddenUnits, blocks, testOut.length);
            File[] merged = mergeFiles(maleFiles, femaleFiles);
            //split files into 5 blocks
            File[] trainBlock = new File[219];
            File[] testBlock = new File[54];
            for(int i = 0; i < 219; i++)
            {
                trainBlock[i] = merged[i];
            }
            for(int i = 219; i < 273; i++)
            {
                testBlock[i - 219] = merged[i];
            }
            backPropagation(trainBlock, blocks, learnRate, answerMap, hiddenUnits, rep);
            test(testBlock, hiddenUnits, blocks);
        }
        else if(test)
        {
            System.out.println("Test");

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
    public static void backPropagation(File[] input, int numBlocks, double rate, Map<File, double[]> output, int num_units, int nreps)
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
        //Map<Key, Double> networkIn = new HashMap<Key, Double>();
        //Map<Key, Double>[] block = new HashMap[numBlocks];
        List<Map<Key,Double>> block = new ArrayList<Map<Key,Double>>();
        int wid = 0;
        int heig = HEIGHT;
        int curRep = 0;
        Map<Key, Double> toAddOne = new HashMap<Key, Double>();
        Map<Key, Double> toAddTwo = new HashMap<Key, Double>();
        Map<Key, Double> toAddThree = new HashMap<Key, Double>();
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
                        if(scanner.hasNextInt() && heig >= 20 && heig < HEIGHT)
                        {
                            if(wid < 30)
                                scanner.next();
                            else if(wid >= 30 && wid < 50)
                            {
                                toAddOne.put(new Key(wid, heig), (double)scanner.nextInt());
                            }
                            else if(wid >= 50 && wid < 70)
                            {
                                toAddTwo.put(new Key(wid, heig), (double)scanner.nextInt());
                            }
                            else if(wid >= 70 && wid < 90)
                            {
                                toAddThree.put(new Key(wid, heig), (double)scanner.nextInt());
                            }
                            else
                                scanner.next();
                            
                            //networkIn.put(new Key(wid, heig), (double)scanner.nextInt());
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
                
                block.add(0, toAddOne);
                block.add(1, toAddTwo);
                block.add(2, toAddThree);

                double[] net_j = new double[num_units];
                double[] out_j = new double[num_units];
                for(int b = 0; b < numBlocks; b++)
                {  
                    for(int h = 0; h < (num_units/numBlocks); h++)
                    {
                        net_j[b*2 + h] = hiddenOut(block.get(b), b*2 + h);
                        //System.out.println(net_j[h]);
                        out_j[b*2 + h] = 1.0 / (1.0 + Math.exp(1.0 - net_j[b*2 + h]));
                    }
                }
                //System.out.println(out_j[0] + " " + out_j[1]);

                double[] net_k = new double[testOut.length];
                double[] out_k = new double[testOut.length];
                for(int k = 0; k < testOut.length; k++)
                {
                    net_k[k] = finalOut(out_j, k);
                    out_k[k] = 1.0 / (1.0 + Math.exp(1.0 - net_k[k]));
                }

                double error_k[] = new double[testOut.length];
                for(int k = 0; k < testOut.length; k++)
                {
                    //System.out.println(file.getName() + " " + answerMap.get(file)[0] + " " + answerMap.get(file)[1]);
                    error_k[k] = out_k[k] * ( 1.0 - out_k[k] ) * ( answerMap.get(file)[k] - out_k[k] );
                }

                double error_j[] = new double[num_units];
                for(int j = 0; j < num_units; j++)
                {
                    double sum = 0;
                    for(int k = 0; k < testOut.length; k++)
                    {
                        sum += error_k[k] * hiddenWeights.get(new Key(j, k));
                    }
                    //System.out.println(j + " against " + num_units);
                    error_j[j] = out_j[j] * ( 1.0 - out_j[j] ) * sum;
                }



                for(int b = 0; b < numBlocks; b++)
                {
                    for(int j = 0; j < (num_units/numBlocks); j++)
                    {
                        double newValue = 0;
                        for(wid = 0; wid < WIDTH; wid++)
                        {
                            for(heig = 0; heig < HEIGHT; heig++)
                            {
                                Map<Key, Double> value = new HashMap<Key, Double>();
                                value = block.get(b);
                                //System.out.println(value.size());
                                if(heig >= 20 && heig < HEIGHT)
                                {
                                    if(wid >= 30 && wid < 50 & b == 0)
                                    {
                                        //System.out.println(wid + " " + heig + " b: " + b);
                                        newValue = rate * error_j[b*2 + j] * value.get(new Key(wid, heig) );
                                        newValue += weights.get(new Key(wid*heig, b*2 + j));
                                        weights.put(new Key(wid*heig, b*2 + j), newValue);
                                    }
                                    else if(wid >= 50 && wid < 70 && b == 1)
                                    {
                                        //System.out.println(wid + " " + heig + " b: " + b);
                                        newValue = rate * error_j[b*2 + j] * value.get(new Key(wid, heig) );
                                        newValue += weights.get(new Key(wid*heig, b*2 + j));
                                        weights.put(new Key(wid*heig, b*2 + j), newValue);
                                    }
                                    else if(wid >= 70 && wid < 90 && b == 2)
                                    {
                                        //System.out.println(wid + " " + heig + " b: " + b);
                                        newValue = rate * error_j[b*2 + j] * value.get(new Key(wid, heig) );
                                        newValue += weights.get(new Key(wid*heig, b*2 + j));
                                        weights.put(new Key(wid*heig, b*2 + j), newValue);
                                    }
                                }
                            }
                        }
                    }
                }

                for(int j = 0; j < num_units; j++)
                {
                    double newValue = 0;
                    for(int k = 0; k < testOut.length; k++)
                    {
                        newValue = rate * error_k[k]  * out_j[j];
                        //System.out.println("Rate: " + rate + " Error: " + error_k[k] + " Out:" + out_j[j] + " Delta: " + newValue );
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

                //System.out.println("Out: " + out_k[0] + " " + out_k[1] + " Answer: " + answerMap.get(file)[0] + " " + answerMap.get(file)[1]);
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

    public static File[] mergeFiles(File[] male, File[] female)
    {
        File[] merged = new File[male.length + female.length];
        int m = 0;
        int f = 0;
        if(male.length > female.length)
        {
            int sections = ((male.length + female.length - 1) / female.length) + 1;
            for(int i = 0; i < male.length + female.length; i++)
            {
                //System.out.println("i: " + i + " m: " + m + " f: " + f + "    SIZES: " + male.length + " " + female.length + " " + sections);
                if(i % sections == 0)
                {
                    merged[i] = female[f];
                    //System.out.println("Put: " + female[f].getName());
                    answerMap.put(female[f], femaleAns);
                    f++;
                }
                else
                {
                    merged[i] = male[m];
                    //System.out.println("Put: " + male[m].getName());
                    answerMap.put(male[m], maleAns);
                    m++;
                }
            }
        }
        else
        {
            int sections = ((female.length + male.length - 1) / male.length) + 1;
            for(int i = 0; i < female.length + male.length; i++)
            {
                if(i % sections == 0)
                {
                    merged[i] = male[m];
                    answerMap.put(male[m], maleAns);
                    m++;
                }
                else
                {
                    merged[i] = female[f];
                    answerMap.put(female[f], femaleAns);
                    f++;
                }
            }
        }
        return merged;
    }

    public static double hiddenOut(Map<Key, Double> in, int curUnit)
    {
        double net = 0;
        int i = 0;
        for(Map.Entry<Key,Double> entry : in.entrySet())
        {
            Key key = entry.getKey();
            Double value = entry.getValue();
            //System.out.println("Calcuted key: " + key.x * key.y);
            net += weights.get(new Key(key.x*key.y, curUnit)) * value;
            //System.out.println(net + " " + weights.get(new Key(i, curUnit)) * value);
            i++;
        }
        //System.out.println(net);
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

    public static void initWeights(int hidden, int numBlocks, int output)
    {
        double start = 0.0001;
        double end = 0.001;
        Random random = new Random();
        
        for(int wid = 0; wid < WIDTH; wid++)
        {
            for(int heig = 0; heig < HEIGHT; heig++)
            {
                if(heig >= 20 && heig < HEIGHT)
                {
                    if(wid < 30)
                        weights.put(new Key(wid*heig, 0), start+(end-start)*random.nextDouble());
                    else if(wid >= 30 && wid < 50)
                    {
                        int blockOne = 0;
                        weights.put(new Key(wid*heig, 0), start+(end-start)*random.nextDouble());
                        weights.put(new Key(wid*heig, 1), start+(end-start)*random.nextDouble());
                    }
                    else if(wid >= 50 && wid < 70)
                    {
                        int blockTwo = 1;
                        weights.put(new Key(wid*heig, 2), start+(end-start)*random.nextDouble());
                        weights.put(new Key(wid*heig, 3), start+(end-start)*random.nextDouble());                           
                    }
                    else if(wid >= 70 && wid < 90)
                    {
                        int blockThree = 2;
                        weights.put(new Key(wid*heig, 4), start+(end-start)*random.nextDouble());
                        weights.put(new Key(wid*heig, 5), start+(end-start)*random.nextDouble());                            
                    }
                    else
                        weights.put(new Key(wid*heig, 5), start+(end-start)*random.nextDouble());
                }
                else
                    weights.put(new Key(wid*heig, 5), start+(end-start)*random.nextDouble());
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
                else if(args[i].equalsIgnoreCase("-crossTest"))
                    crossTest = true;
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

    public static void test(File[] in, int num_units, int numBlocks)
    {
        Scanner scanner;
        //Map<Key, Double> blockOne = new HashMap<Key, Double>();
        //Map<Key, Double> blockTwo = new HashMap<Key, Double>();
        //Map<Key, Double> networkIn = new HashMap<Key, Double>();
        //Map<Key, Double>[] block = new HashMap[numBlocks];
        List<Map<Key,Double>> block = new ArrayList<Map<Key,Double>>();
        int wid = 0;
        int heig = HEIGHT;
        int curRep = 0;
        Map<Key, Double> toAddOne = new HashMap<Key, Double>();
        Map<Key, Double> toAddTwo = new HashMap<Key, Double>();
        Map<Key, Double> toAddThree = new HashMap<Key, Double>();
                //The below code steps through all files from a given folder.
        //This is the FOR EACH INPUT step
        for(File file : in)
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
                    if(scanner.hasNextInt() && heig >= 20 && heig < HEIGHT)
                    {
                        if(wid < 30)
                            scanner.next();
                        else if(wid >= 30 && wid < 50)
                        {
                            toAddOne.put(new Key(wid, heig), (double)scanner.nextInt());
                        }
                        else if(wid >= 50 && wid < 70)
                        {
                            toAddTwo.put(new Key(wid, heig), (double)scanner.nextInt());
                        }
                        else if(wid >= 70 && wid < 90)
                        {
                            toAddThree.put(new Key(wid, heig), (double)scanner.nextInt());
                        }
                        else
                            scanner.next();
                        
                        //networkIn.put(new Key(wid, heig), (double)scanner.nextInt());
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
            
            block.add(0, toAddOne);
            block.add(1, toAddTwo);
            block.add(2, toAddThree);

            double[] net_j = new double[num_units];
            double[] out_j = new double[num_units];
            for(int b = 0; b < numBlocks; b++)
            {  
                for(int h = 0; h < (num_units/numBlocks); h++)
                {
                    net_j[b*2 + h] = hiddenOut(block.get(b), b*2 + h);
                    //System.out.println(net_j[h]);
                    out_j[b*2 + h] = 1.0 / (1.0 + Math.exp(1.0 - net_j[b*2 + h]));
                }
            }
            //System.out.println(out_j[0] + " " + out_j[1]);

            double[] net_k = new double[testOut.length];
            double[] out_k = new double[testOut.length];
            for(int k = 0; k < testOut.length; k++)
            {
                net_k[k] = finalOut(out_j, k);
                out_k[k] = 1.0 / (1.0 + Math.exp(1.0 - net_k[k]));
            }
            System.out.println(out_k[0] + " " + out_k[1]);

            if(out_k[0] >= 0.0 && out_k[0] <= 0.49 && out_k[1] >= 0.5 && out_k[1] <= 1)
            {
                System.out.println("Prediction: Male. Confidence: ");
            }
            if(out_k[1] >= 0.0 && out_k[1] <= 0.49 && out_k[0] >= 0.5 && out_k[0] <= 1)
            {
                System.out.println("Prediction: Female. Confidence: ");
            }

        }
    }

    public static void saveWeights()
    {
        //save the weights and hiddenWeights
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


