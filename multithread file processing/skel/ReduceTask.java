import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class ReduceTask implements Runnable{
    private final String filename;
    private final Vector<MapDataSet> map_results;

    Vector<String> words_max_len = new Vector<String>();
    int max_len = 0;
    HashMap<Integer, Integer> len_words_dataset = new HashMap<Integer, Integer>();

    public ReduceTask(String filename, Vector<MapDataSet> map_results) {
        this.filename = filename;
        this.map_results = map_results;
    }

    public int getFiboAtIndex(int index){
        int n1 = 0;
        int n2 = 1;
        int n3 = 0;
        if(index == 0){
            return n1;
        }
        if(index == 1){
            return n2;
        }
        for(int i = 2; i<= index; i++){
            n3 = n1 + n2;
            n1 = n2;
            n2 = n3;
        }
        return n3;
    }

    @Override
    public void run() {

        for (MapDataSet mds : this.map_results) {

            //combine vectors of words with max length
            if(mds.words_max_len.elementAt(0).length() > max_len){
                max_len = mds.words_max_len.elementAt(0).length();
                words_max_len.clear();
                words_max_len.addAll(mds.words_max_len);
            } else if (mds.words_max_len.elementAt(0).length() == max_len) {
                words_max_len.addAll(mds.words_max_len);
            }

            //combine hash maps
            for(Map.Entry<Integer, Integer> entry : mds.len_words_dataset.entrySet()){
                int key = entry.getKey();
                int val = entry.getValue();

                if(len_words_dataset.containsKey(key)) {
                    len_words_dataset.put(key, len_words_dataset.get(key) + val);
                } else {
                    len_words_dataset.put(key, val);
                }
            }
        }

        //processing stage, compute rank of file
        float rank = 0;
        float value = 0;
        float total_no_words = 0;
        int max_len_word = 0;
        int no_max_len_word = 0;
        for(Map.Entry<Integer, Integer> entry : len_words_dataset.entrySet()){
            int len_word = entry.getKey();
            int no_words = entry.getValue();
            value += this.getFiboAtIndex(len_word + 1) * no_words;
            total_no_words += no_words;

            if(max_len_word < len_word) {
                max_len_word = len_word;
                no_max_len_word = no_words;
            }
        }
        rank = value / total_no_words;
        //System.out.println(filename + " rank: " + String.format("%.2f", rank));

        synchronized (Tema2.reduce_result){
            Tema2.reduce_result.add(new FRMlwSet(Tema2.path_to_fileName.get(filename) , rank, max_len_word, no_max_len_word));
        }

        Tema2.inQueue.decrementAndGet();
        if(Tema2.inQueue.get() == 0)
            Tema2.tpe_reduce.shutdown();
    }
}
