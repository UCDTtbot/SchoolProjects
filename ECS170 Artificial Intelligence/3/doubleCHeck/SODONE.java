import java.io.*;
//import java.util.Scanner;
//import java.util.ArrayList;
//import java.util.List;
//import java.util.Random;
//import java.util.HashMap;
import java.util.*;
import java.lang.Math;

public class SODONE
{
    public static File[] maleFiles;
    public static File[] femaleFiles;
    public static File[] testFiles;
    public static boolean test = false;
    public static boolean train = false;
    public static boolean crossTest = false;
    public static double maleAns = 0.8;
    public static double femaleAns = 0.2;
    public static final int WIDTH = 128;
    public static final int HEIGHT = 120;

    public static final double learnRate = 0.05;
    public static final int rep = 50;
    public static final int inputUnits = WIDTH * HEIGHT + 1;
    public static final int hiddenUnits = 5;
    public static final int outputUnits = 1;
    public static final Map<File, Double> answerMap = new HashMap<File, Double>();
    public static double[][] weights = new double[inputUnits][hiddenUnits];
    public static double[][] hiddenWeights = new double[hiddenUnits][outputUnits];
	
    /// Program startup function.
	public static void main(String[] args)
	{

        maleFiles = openFiles("/Male", "/male");
        femaleFiles = openFiles("/Female", "/female");
        testFiles = openFiles("/Test", "/test");

        parseArgs(args);

        if(train)
        {
            System.out.println("Train");
            File[] merged = mergeFiles(maleFiles, femaleFiles);
            //File[] weaveMerged = weaveMergeFiles(maleFiles, femaleFiles);
            //System.out.println("Size of merged encoding: " + mergedEncoding.size() + " Should be: " + merged.length);
            initWeights(inputUnits, hiddenUnits, outputUnits);
            //input, double rate, int in_units, int hidden_units, int out_units, int nreps
            backPropagation(merged, learnRate, inputUnits, hiddenUnits, outputUnits, rep);
            saveWeights(weights, hiddenWeights, inputUnits, hiddenUnits, outputUnits);
            //double correct = test(testFiles, inputUnits, hiddenUnits, outputUnits, true);
            //for(int i = 0; i < hiddenUnits; i++)
              //  System.out.println(hiddenWeights[i][0]);
        }
        else if(crossTest)
        {
            System.out.println("Cross Test");
            File[] merged = mergeFiles(maleFiles, femaleFiles);
            //File[] weaveMerged = weaveMergeFiles(maleFiles, femaleFiles);
            int blockSize = merged.length / 5;
            
            int testSize = blockSize;
            int trainSize = blockSize * 4;
            
            //System.out.println(merged.length + " | " + blockSizes + " | " + trainBlock.length + " | " + testBlock.length);
            double[] runAcc = new double[5];
            //*********ADDED TRAINING ACCURACY ARRAY************
            double[] trainAcc = new double[5];
            //*********RENAMED VARIABLE************
            double meanTestAcc = 0;
            //*********MEAN TRAIN ACCURACY************
            double meanTrainAcc = 0;
            //*********ADDED MEAN TRAINING ARRAY FOR EACH BATCH************
            double meanTrainingAcc = 0;
            //double totalMeanTrainingAcc = 0;
            //*********RENAMED************                        
            double meanTestingAcc = 0;
            //double totalMeanTestAcc = 0;
            //for standard deviation
            double[] squaredTrain = new double[5];
            double[] squaredTest = new double[5];
            double diffTrain = 0, diff = 0, trainSum = 0, sdTrain = 0, testSum = 0, sdTest = 0;

            //Train Block 1
            System.out.println("Running cross test 10 times:");
            for(int totalTest = 0; totalTest < 10; totalTest++)
            {
                System.out.println("Batch #" + totalTest);
                for(int curTest = 0; curTest < 5; curTest++)
                {
                    File[] trainBlock = new File[trainSize];
                    File[] testBlock = new File[testSize];
                    if(curTest == 0)
                    {
                        for(int i = 0; i < trainSize; i++)
                        {
                             trainBlock[i] = merged[i];
                        }
                        for(int i = trainSize; i < trainSize+testSize; i++)
                        {
                             testBlock[i - trainSize] = merged[i];
                        }
                    }
                    else if(curTest == 1)
                    {  
                        for(int i = 0; i < blockSize * 3; i++)
                        {
                            trainBlock[i] = merged[i];
                        }
                        for(int i = blockSize * 3; i < blockSize * 4; i++)
                        {
                            testBlock[i-blockSize*3] = merged[i];
                        }
                        for(int i = blockSize*4; i < blockSize*5; i++)
                        {
                            trainBlock[i-blockSize] = merged[i];
                        }
                    }
                    else if(curTest ==  2)
                    {
                        for(int i = 0; i < blockSize * 2; i++)
                        {
                            trainBlock[i] = merged[i];
                        }
                        for(int i = blockSize * 2; i < blockSize * 3; i++)
                        {
                            testBlock[i-blockSize*2] = merged[i];
                        }
                        for(int i = blockSize*3; i < blockSize*5; i++)
                        {
                            trainBlock[i-blockSize] = merged[i];
                        }
                        
                    }
                    else if(curTest == 3)
                    {
                        for(int i = 0; i < blockSize * 1; i++)
                        {
                            trainBlock[i] = merged[i];
                        }
                        for(int i = blockSize * 1; i < blockSize * 2; i++)
                        {
                            testBlock[i-blockSize*1] = merged[i];
                        }
                        for(int i = blockSize*2; i < blockSize*5; i++)
                        {
                            trainBlock[i-blockSize] = merged[i];
                        }
                    }
                    else if(curTest == 4)
                    {
                        for(int i = 0; i < trainSize; i++)
                        {
                             trainBlock[i] = merged[i + blockSize];
                        }
                        for(int i = 0; i < testSize; i++)
                        {
                             testBlock[i] = merged[i];
                        }
                    }
                    else
                        System.out.println("Error");

                    initWeights(inputUnits, hiddenUnits, outputUnits);
                    System.out.println("Cross Test: " + curTest + " Training Size: " + trainBlock.length + " Test Size: "  + testBlock.length);
                    backPropagation(trainBlock, learnRate, inputUnits, hiddenUnits, outputUnits, rep);
                    //*********FED TRAINING DATA INTO TRAIN ACCURACY ARRAY************
                    trainAcc[curTest] = test(trainBlock, inputUnits, hiddenUnits, outputUnits, false);
                    //*********TRAIN PERCENT ERROR************
                    System.out.println("Train Perc Error: " + trainAcc[curTest] * 100.0);
                    //test accuracy array
                    runAcc[curTest] = test(testBlock, inputUnits, hiddenUnits, outputUnits, false);
                    System.out.println("Test Perc Error: " + runAcc[curTest] * 100.0);
                }

                //*********ADDED A RESET. OTHERWISE MEAN ERRORS GO OVER 100************
                //*********FOR LOOP TO GET SUM OF TRAINING OUTPUTS************
                meanTrainAcc = 0;
                for(int a = 0; a < 5; a++) {
                    meanTrainAcc += trainAcc[a];
                }
                //*********GETS AVERAGE************
                meanTrainingAcc = meanTrainAcc / 5.0;

                //*********SAME AS ABOVE************
                //*********CHANGED UPPER BOUND TO 5 INSTEAD OF 4************
                meanTestAcc = 0;
                for(int u = 0; u < 5; u++)
                {
                    meanTestAcc += runAcc[u];
                }
                meanTestingAcc = meanTestAcc / 5.0;

                //calculating standard deviation
                diffTrain = 0;
                for(int e = 0; e < 5; e++) {
                    diffTrain = trainAcc[e] - meanTrainingAcc;
                    squaredTrain[e] = diffTrain * diffTrain;
                }

                diff = 0;
                for(int t = 0; t < 5; t++) {
                    diff = runAcc[t] - meanTestingAcc;
                    squaredTest[t] = diff * diff;
                }

                trainSum = 0;
                for(int i = 0; i < 5; i++) {
                    trainSum += squaredTrain[i];
                }
                sdTrain = Math.sqrt(trainSum / 5.0);

                testSum = 0;
                for(int j = 0; j < 5; j++) {
                    testSum += squaredTest[j];
                }
                sdTest = Math.sqrt(testSum / 5.0);

                // System.out.println("Mean Training Accuracy: " + meanTrainAcc);
                // System.out.println("Mean Testing Accuracy: " + meanTestAcc);
                //*********AVERAGE PERCENT ERROR IN TRAINING ADDED************
                System.out.println("Mean percent error in training: " + totalTest + " is: " + meanTrainingAcc);
                System.out.println("Standard deviation in training: " + sdTrain);
                System.out.println("Mean percent error in testing: " + totalTest + " is: " + meanTestingAcc);
                System.out.println("Standard deviation in testing: " + sdTest);
            }
        }
        else if(test)
        {
            System.out.println("Test");
            //System.out.println("Size of merged encoding: " + mergedEncoding.size() + " Should be: " + merged.length);
            //backPropagation(femaleEncoding, learnRate, hiddenUnits, rep);
            //backPropagation(maleEncodig, learnRate, hiddenUnits, rep);
            readWeights(inputUnits, hiddenUnits, outputUnits);
            //input, double rate, int in_units, int hidden_units, int out_units, int nreps
            test(testFiles, inputUnits, hiddenUnits, outputUnits, true);
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
    public static void backPropagation(File[] input, double rate, int in_units, int hidden_units, int out_units, int nreps)
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
        //Map<Key, Double> blockOne = new HashMap<Key, Double>();
        //Map<Key, Double> blockTwo = new HashMap<Key, Double>();
        int curRep = 0;
                //The below code steps through all files from a given folder.
        //This is the FOR EACH INPUT step
        System.out.println("Running Backprop");
        boolean terminate = false;
        while(!terminate)
        {
            curRep++;
            for(File file : input)
            {
                double[] pixelBlock = getPix(file);
                double answer = answerMap.get(file);
                //System.out.println("File: " + file.getName() + " Answer: " + answer);

                double[] net_j = new double[hidden_units];
                double[] out_j = new double[hidden_units];

                //System.out.println(pixelBlock.size());

                for(int h = 0; h < hidden_units; h++)
                {
                    net_j[h] = hiddenNet(pixelBlock, h);
                    out_j[h] = 1.0 / (1.0 + Math.exp(1.0 - net_j[h]));
                    //System.out.println("Output for hidden unit: " + h + " = " + out_j[h]);
                }
                //System.out.println(out_j[0] + " " + out_j[1]);

                double net_k = 0.0;
                double out_k = 0.0;
                // for(int k = 0; k < out_units; k++)
                // {
                //     net_k[k] = finalOut(out_j, k);
                //     out_k[k] = 1.0 / (1.0 + Math.exp(1.0 - net_k[k]));
                //     //System.out.println("Output for out unit: " + k + " = " + out_k[k]);
                // }
                net_k = finalNet(out_j, 0);
                out_k = 1.0 / (1.0 + Math.exp(1.0 - net_k));
                //System.out.println("Out for file: " + file.getName() + " is : " + out_k);
                //Network error
                // for(int k = 0; k < out_units; k++)
                // {
                //     double netError = 0;
                //     netError += ( Math.pow(answer[k] - out_k[k], 2) ) / 2.0;
                //     //System.out.println("Got: " + out_k[k] + " Answer: " + answer[k] + " Difference = " + (answer[k] - out_k[k]) );
                //     //System.out.println("Net Error for out: " + k + " = " + netError);
                // }

                double error_k = 0;
                //for(int k = 0; k < out_units; k++)
                // {
                //     //System.out.println(file.getName() + " " + answerMap.get(file)[0] + " " + answerMap.get(file)[1]);
                //     error_k[k] = out_k[k] * ( 1.0 - out_k[k] ) * ( answer[k] - out_k[k] );
                //     //System.out.println("Error for output unit: " + k + " = " + error_k[k] );
                // }
                error_k = out_k * ( 1.0 - out_k ) * (answer - out_k);

                double error_j[] = new double[hidden_units];
                for(int j = 0; j < hidden_units; j++)
                {
                    double sum = 0;
                    // for(int k = 0; k < out_units; k++)
                    // {
                    //    sum += error_k[k] * hiddenWeights[j][k];
                    // }
                    sum = error_k * hiddenWeights[j][0];
                    //System.out.println("HiddenWeight " + j + " " + hiddenWeights[j][0]);
                    //System.out.println(j + " against " + num_units);
                    error_j[j] = out_j[j] * ( 1.0 - out_j[j] ) * sum;
                    ///error_j[j] = out_j[j] * (1.0 - out_j[j]) * error_k * hiddenWeights[j][0];
                }
                for(int j = 0; j < hidden_units; j++)
                {
                    double newValue = 0;
                    // for(int k = 0; k < out_units; k++)
                    // {
                    //     newValue = rate * error_k[k]  * out_j[j];
                    //     //System.out.println("Rate: " + rate + " Error: " + error_k[k] + " Out:" + out_j[j] + " Delta: " + newValue );
                    //     newValue += hiddenWeights[j][k];
                    //     //System.out.println("Hidden Unit " + j + " To Output Unit " + k + " Old weight: " + hiddenWeights[j][k] + " New Weight: " + newValue);
                    //     hiddenWeights[j][k] = newValue;
                    // }
                    newValue = rate * error_k * out_j[j];
                    newValue += hiddenWeights[j][0];
                    //System.out.println("Change in hidden weight " + j + " is: " + (hiddenWeights[j][0] - newValue) );
                    hiddenWeights[j][0] = newValue;
                }

                for(int i = 0; i < inputUnits; i++)
                {
                    double newValue = 0;
                    for(int j = 0; j < hidden_units; j++)
                    {
                        newValue = rate * error_j[j] * pixelBlock[i];
                        //newValue += weights.get(new Key(i, j));
                        newValue += weights[i][j];
                        //weights.put(new Key(i, j), newValue);
                        weights[i][j] = newValue;
                    }
                }
                
                //System.out.println("Out: " + out_k[0] + " " + out_k[1] + " Answer: " + answer[0] + " " + answer[1]);
                //while(true);
            }
            if(curRep == nreps)
                terminate = true;
        }

    }

    public static double test(File[] input, int in_units, int hidden_units, int out_units, boolean trueTest)
    {
        //Map<Key, Double> blockOne = new HashMap<Key, Double>();
        //Map<Key, Double> blockTwo = new HashMap<Key, Double>();
        //Map<Key, Double> networkIn = new HashMap<Key, Double>();
        double totalTests = input.length;
        double numCorrect = 0;
        for(File file : input)
        {
            double[] pixelBlock = getPix(file);
            //System.out.println("File: " + file.getName() + " Answer: " + answer);

            double[] net_j = new double[hidden_units];
            double[] out_j = new double[hidden_units];

            //System.out.println(pixelBlock.size());

            for(int h = 0; h < hidden_units; h++)
            {
                net_j[h] = hiddenNet(pixelBlock, h);
                out_j[h] = 1.0 / (1.0 + Math.exp(1.0 - net_j[h]));
                //System.out.println("Output for hidden unit: " + h + " = " + out_j[h]);
            }
            //System.out.println(out_j[0] + " " + out_j[1]);

            double net_k = 0.0;
            double out_k = 0.0;
            // for(int k = 0; k < out_units; k++)
            // {
            //     net_k[k] = finalOut(out_j, k);
            //     out_k[k] = 1.0 / (1.0 + Math.exp(1.0 - net_k[k]));
            //     //System.out.println("Output for out unit: " + k + " = " + out_k[k]);
            // }
            net_k = finalNet(out_j, 0);
            out_k = 1.0 / (1.0 + Math.exp(1.0 - net_k));
            if(trueTest)
            {
                //double normGuess = (Math.sqrt( Math.pow(out_k[0], 2) + Math.pow(out_k[1], 2) ));
                //System.out.println("Ans: " + out_k);
                if(out_k >= 0.5)
                {
                    double err = Math.abs(1.0 - (out_k / maleAns));
                    double conf = 1.0 - err;
                    System.out.print("Prediction: MALE Confidence: ");
                    System.out.printf("%.2f", conf*100); System.out.print("%");
                    System.out.println();
                }
                else if(out_k < 0.5)
                {
                    double err = Math.abs(1.0 - (femaleAns / out_k));
                    double conf = Math.abs(1.0 - err);
                    System.out.print("Prediction: FEMALE Confidence: ");
                    System.out.printf("%.2f", conf*100); System.out.print("%");
                    System.out.println();
                }
            }
            else
            {
                double answer = answerMap.get(file);
                if(out_k >= 0.5)
                {
                   // System.out.println("Prediction: Male. Ans: " + out_k);
                    if(answer == maleAns)
                    {
                         //System.out.println("Correct, Male");
                         numCorrect++;
                    }
                }
                else if(out_k < 0.5)
                {
                    //System.out.println("Prediction: Female. Ans: " + out_k);
                    if(answer == femaleAns)
                    {
                        //System.out.println("Correct, Female");
                        numCorrect++;
                    }
                }
                else
                    ;
            }

            
        }

        double percentWrong = (totalTests - numCorrect)/totalTests * 100.0;
        return percentWrong;
    }


    public static File[] openFiles(String path1, String path2)
    {
        String filePath1 = new File("").getAbsolutePath();
        String filePath2 = new File("").getAbsolutePath();
        filePath1 = filePath1.concat(path1);
        filePath2 = filePath2.concat(path2);
        File folder1 = new File(filePath1);
        File folder2 = new File(filePath2);
        System.out.print(path1 + " exists: ");
        System.out.println(folder1.exists());
        System.out.print(path2 + " exists: ");
        System.out.println(folder2.exists());
        File[] listOfFiles;
        if(folder1.exists())
            listOfFiles = folder1.listFiles();
        else
            listOfFiles = folder2.listFiles();

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
    public static File[] weaveMergeFiles(File[] male, File[] female)
    {
        //File[] merged = new File[male.length + female.length];
        File[] merged = new File[male.length + female.length*(male.length/female.length)];
        //System.out.println(male.length + " | " + female.length + " | " + (male.length + female.length*(male.length/female.length)) );
        int m = 0;
        int f = 0;

        if(male.length > female.length)
        {
            m = 0;
            f = 0;
            for(int i = 0; i < (male.length + female.length*(male.length/female.length)); i++)
            {
                merged[i] = male[m];
                answerMap.put(male[m], maleAns);
                m++;
                i++;
                if(i < male.length + female.length*(male.length/female.length))
                {
                    merged[i] = female[f];
                    answerMap.put(female[f], femaleAns);
                    f++;
                    if(f >= female.length)
                        f = 0;
                }
                
                //System.out.println(female.length + " current f: " + f + " current i " + i);
            }
        }

        // for(File fi : merged)
        // {
        //     System.out.println(answerMap.get(fi)[0] + " " + answerMap.get(fi)[1]);
        // }
        return merged;
    }

    public static double hiddenNet(double[] in, int curUnit)
    {
        //Bias
        double net = 0.0;
        for(int i = 0; i < in.length; i++)
        {
            double value = in[i];
            //System.out.println(key + " to " + curUnit);
            //System.out.println(key.x + " " + key.y + " " + value + " " + weights.get(new Key(i, curUnit)));
            //net += weights.get(new Key(key, curUnit)) * value;
            net += weights[i][curUnit] * value;
            //System.out.println(net + " " + weights.get(new Key(i, curUnit)) * value);
        }
        //System.out.println("Net for hidden unit: " + curUnit + " = " + net);
        return net;
    }

    public static double finalNet(double[] in, int k)
    {
        //Bias
        //System.out.println("Num of hidden units inputed: " + in.length + ", current output unit " + k);
        double net = 0.0;
        for(int j = 0; j < in.length; j++)
        {
            //net += hiddenWeights.get(new Key(j, k)) * in[j];
            net += hiddenWeights[j][k] * in[j];
        }
        //System.out.println("Net for output unit: " + k + " = " + net);
        return net;
    }

    public static void initWeights(int input, int hidden, int output)
    {
        double start = -0.05;
        double end = 0.05;
        Random random = new Random();
        
        //System.out.println(input + " " + hidden + " " + output);
        for(int i = 0; i < input; i++)
        {
            for(int j = 0; j < hidden; j++)
            {
                //weights.put(new Key(i, j), start+(end-start)*random.nextDouble());
                weights[i][j] = start + (end-start) * random.nextDouble();
                //weights.put(new Key(i, j), 0.0);
            }
        }

        for(int j = 0; j < hidden; j++)
        {
            for(int k = 0; k < output; k++)
            {
                //hiddenWeights.put(new Key(j,k), start+(end-start)*random.nextDouble());
                hiddenWeights[j][k] = start + (end - start) * random.nextDouble();
            }
        }
        // for(Map.Entry<Key,Double> entry : weights.entrySet())
        // {
        //     Key key = entry.getKey();
        //     Double value = entry.getValue();
        //     System.out.println("Weight: " + key.x + " " + key.y + " V: " + value);
        // }
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

    public static double[] getPix(File file)
    {
        //double[][] values = new double[(WIDTH/32)][(HEIGHT/30)];
        double[] pixels = new double[WIDTH*HEIGHT + 1];
        int i = 0, j = 0;
        int z = 1;
        pixels[0] = 1;
        try
        {
            Scanner scanner = new Scanner(file);
            while(scanner.hasNext())
            {
                if(scanner.hasNextInt())
                {
                    pixels[z] = (double)scanner.nextInt() / 255.0;
                    z++;
                }
                else
                {
                    scanner.next();
                    z++;
                }
            }
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println(fnfe.getMessage());
        }
        
        
        return pixels;
    }

    public static void saveWeights(double[][] weigh, double[][] hidweigh, int num_input, int num_hidden, int num_output)
    {
        //save the weights and hiddenWeights
        String filePath = new File("").getAbsolutePath();
        filePath = filePath.concat("/weights.txt");
        //System.out.println(filePath);
        try
        {
            PrintWriter out = new PrintWriter(filePath, "UTF-8");
            for(int i = 0; i < num_input; i++)
            {
                for(int j = 0; j < num_hidden; j++)
                {
                    out.println(weigh[i][j]);             
                }
            }   
            out.close();
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println(fnfe.getMessage());
        }
        catch(UnsupportedEncodingException uee)
        {
            System.out.println(uee.getMessage());
        }

        String hiddenPath = new File("").getAbsolutePath();
        hiddenPath = hiddenPath.concat("/hiddenWeights.txt");
        //System.out.println(filePath);
        try
        {
            PrintWriter out = new PrintWriter(hiddenPath, "UTF-8");
            for(int i = 0; i < num_hidden; i++)
            {
                for(int j = 0; j < num_output; j++)
                {
                    out.println(hidweigh[i][j]);             
                }
            }   
            out.close();
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println(fnfe.getMessage());
        }
        catch(UnsupportedEncodingException uee)
        {
            System.out.println(uee.getMessage());
        }
    }

    public static void readWeights(int num_input, int num_hidden, int num_output)
    {
        String weightPath = new File("").getAbsolutePath();
        String hiddenPath = new File("").getAbsolutePath();
        weightPath = weightPath.concat("/weights.txt");
        hiddenPath = hiddenPath.concat("/hiddenWeights.txt");
        File weightFile = new File(weightPath);
        File hiddenFile = new File(hiddenPath);

        System.out.print("/weights.txt exists: ");
        System.out.println(weightFile.exists());

        System.out.print("/hiddenWeights.txt exists: ");
        System.out.println(hiddenFile.exists());

        /*
            for(int i = 0; i < num_input; i++)
            {
                for(int j = 0; j < num_hidden; j++)
                {
                    out.println(weigh[i][j]);             
                }
            }   
        */
        try
        {
            Scanner weig = new Scanner(weightFile);
            Scanner hidd = new Scanner(hiddenFile);
            int i = 0;
            int j = 0;
            while(weig.hasNextDouble())
            {
                weights[i][j] = weig.nextDouble();
                j++;
                if(j == num_hidden)
                {
                    i++;
                    j = 0;
                }
            }
            //System.out.println("Read in " + i + " " + j + " weights");
            i = 0;
            j = 0;
            while(hidd.hasNextDouble())
            {
                hiddenWeights[i][j] = hidd.nextDouble();
                System.out.println("Weight: " + i + " " + hiddenWeights[i][j]);
                i++;
                if(i == num_hidden)
                {
                    i = 0;
                    j++;
                }
            }
            //System.out.println("Read in " + i + " " + j +  " hidden weights" );
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println(fnfe.getMessage());
        }   
    
    }

    public static void writeOut(Map<Integer, Double> inputs, String name)
    {
        String filePath = new File("").getAbsolutePath();
        filePath = filePath.concat("/" + name);
        //System.out.println(filePath);
        try
        {
            PrintWriter out = new PrintWriter(filePath, "UTF-8");
            for(Map.Entry<Integer,Double> entry : inputs.entrySet())
            {
                int pixNum = entry.getKey();
                double value = entry.getValue() * 255;
                int v = (int)value;
                out.println(v);             
            }   
            out.close();
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println(fnfe.getMessage());
        }
        catch(UnsupportedEncodingException uee)
        {
            System.out.println(uee.getMessage());
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

