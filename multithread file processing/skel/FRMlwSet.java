// set of File, Rank, Max len word
public class FRMlwSet implements Comparable<FRMlwSet> {
    public String fileName;
    public float rank;
    public int max_len_word;
    public int no_max_len_word;

    public FRMlwSet (String fileName, float rank, int max_len_word, int no_max_len_word){
        this.fileName = new String(fileName);
        this.rank = rank;
        this.max_len_word = max_len_word;
        this.no_max_len_word = no_max_len_word;
    }

    @Override
    public int compareTo(FRMlwSet o) {
        if (rank > o.rank) {
            return -1;
        } else {
            return 1;
        }
    }
}
