import java.util.HashMap;
import java.util.Vector;

public class MapDataSet {
    public HashMap<Integer, Integer> len_words_dataset = new HashMap<Integer, Integer>();;
    public Vector<String> words_max_len = new Vector<String>();

    public MapDataSet(HashMap<Integer, Integer> len_words_dataset, Vector<String> words_max_len) {
        this.len_words_dataset.putAll(len_words_dataset);
        this.words_max_len.addAll(words_max_len);
    }

    public String toString(){
        return "!" + len_words_dataset.toString() + words_max_len.toString() + "!";
    }
}
