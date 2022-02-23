import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema2 {
    static int no_workers;
    static String file_in;
    static String file_out;
    static ExecutorService tpe_map;
    static ExecutorService tpe_reduce;
    static AtomicInteger inQueue = new AtomicInteger(0);
    static String separators = ";:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| \t\r\n\0";

    static HashMap<String, Vector<MapDataSet>> map_results = new HashMap<String, Vector<MapDataSet>>();
    static HashMap<String, String>path_to_fileName = new HashMap<String, String>();
    static ArrayList<FRMlwSet> reduce_result = new ArrayList<FRMlwSet>();

    public static void main(String[] args) {

        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }
        //get command line args
        no_workers = Integer.parseInt(args[0]);
        file_in = args[1];
        file_out = args[2];
        tpe_map = Executors.newFixedThreadPool(no_workers);
        tpe_reduce = Executors.newFixedThreadPool(no_workers);

        Scanner sc = null;
        //parse file_in input
        try {
            sc = new Scanner(new File(file_in));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        //parse input file contents
        int fragment_dim = Integer.parseInt(sc.nextLine());
        int no_docks = Integer.parseInt(sc.nextLine());

        String f_name = new String();
        //Map stage
        for(int i = 0; i < no_docks; i++){
            //read file path
            f_name = sc.nextLine();
            String f = "../" + f_name;

            //copute file name
            StringTokenizer fName_st = new StringTokenizer(f_name, "/");
            String s = new String();
            while(fName_st.hasMoreTokens()){
                s = fName_st.nextToken();
            }

            path_to_fileName.put(f, s);
            //compute file dimension
            long f_dim = 0;
            Path f_path = Paths.get(f);
            try {
                f_dim = Files.size(f_path);
            } catch (IOException e) {
                e.printStackTrace();
            }

            //prepare tasks for Map stage
            //each Map task is sent doc_name, doc_offset from start, fragment dimension
            long offset = 0;
            //this needs to be in a loop! make sure all the file is read
            while(offset < f_dim){
                inQueue.incrementAndGet();
                map_results.put(f ,new Vector<MapDataSet>());
                tpe_map.submit(new MapTask(f, offset, fragment_dim));
                offset += fragment_dim;
            }
        }
        //wait for all to terminate
        try {
            tpe_map.awaitTermination(60, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        //Reduce stage
        inQueue.set(0);
        for(Map.Entry<String, Vector<MapDataSet>> entry : map_results.entrySet()){
            String file_name = entry.getKey();
            Vector<MapDataSet> map_stage_data = entry.getValue();

            inQueue.incrementAndGet();
            tpe_reduce.submit(new ReduceTask(file_name, map_stage_data));
        }


        try {
            tpe_reduce.awaitTermination(60, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Collections.sort(reduce_result);
        try {
            PrintWriter pw = new PrintWriter(new FileWriter(file_out));
            for(FRMlwSet item : reduce_result){
                pw.printf("%s,%.2f,%d,%d\n", item.fileName, item.rank, item.max_len_word, item.no_max_len_word);
            }
            pw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
// java Tema2 3 ../tests/in/test0.txt out