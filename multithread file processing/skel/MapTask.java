import java.io.*;
import java.util.HashMap;
import java.util.StringTokenizer;
import java.util.Vector;

public class MapTask implements Runnable{
    private final String file_name;
    private final long offset;
    private final int frag_dim;
    HashMap<Integer, Integer> len_words_dataset = new HashMap<>();
    Vector<String> words_max_len = new Vector<>();

    public MapTask(String file_name, long offset, int frag_dim) {
        this.file_name = file_name;
        this.offset = offset;
        this.frag_dim = frag_dim;
    }

    @Override
    public void run() {
        //System.out.println("Got sent the params " + file_name + " " + offset + " " + frag_dim);
        RandomAccessFile raf = null;
        byte[] bytes = new byte[frag_dim];
        String read_data = null;
        long start_offset, end_offset;
        long file_size;
        boolean nothing_to_read = false;

        try {
            raf = new RandomAccessFile(new File(file_name), "r");
            file_size = raf.length();
            start_offset = offset;
            end_offset = offset + frag_dim - 1;
            if(end_offset > file_size) {
                end_offset = file_size - 1;
            }

            //compute starting offset(only if it's not start of file)
            //check if starting position is at start of word
            if(start_offset > 0) {
                while(true){
                    if(start_offset >= file_size || start_offset >= end_offset){
                        break;
                    }
                    raf.seek(start_offset - 1);
                    byte[] check_bytes = new byte[2];
                    raf.read(check_bytes);
                    String s = new String(check_bytes);
                    if(Tema2.separators.contains(String.valueOf(s.charAt(0))) && !Tema2.separators.contains(String.valueOf(s.charAt(1)))){
                        break;
                    } else {
                        start_offset++;
                    }
                }
            }

            //compute end offset (only if the end offset is not above EOF)
            //check if end of fragment is end of word
            if(end_offset < file_size && end_offset > start_offset){
                while(true){
                    if(end_offset >= file_size){
                        break;
                    }
                    raf.seek(end_offset);
                    byte[] check_bytes = new byte[2];
                    raf.read(check_bytes);
                    String s = new String(check_bytes);
                    if(Tema2.separators.contains(String.valueOf(s.charAt(0)))){
                        break;
                    }
                    if(Tema2.separators.contains(String.valueOf(s.charAt(1))) && !Tema2.separators.contains(String.valueOf(s.charAt(0)))) {
                        break;
                    } else {
                        end_offset++;
                    }
                }
            }

            if(start_offset >= end_offset)
                nothing_to_read = true;

            if(!nothing_to_read){
                raf.seek(start_offset);
                bytes = new byte[(int)(end_offset - start_offset + 1)];
                raf.read(bytes);
                read_data = new String(bytes);

                //now pass the string through a StringTokenizer and compute
                // 1) the data set of <K, V>
                //      K is the length of words that appear
                //      V is the number of words with that length
                // 2) list of max length words
                // => output : { Doc_name ; data set ; list words max len }

                int max_len = 0;
                StringTokenizer st = new StringTokenizer(read_data, Tema2.separators);

                while(st.hasMoreTokens()){
                    String word = new String(st.nextToken());
                    int word_len = word.length();

                    //save words with max length
                    if(max_len < word_len){
                        words_max_len.clear();
                        words_max_len.add(word);
                        max_len = word_len;
                    } else if(max_len == word_len){
                        words_max_len.add(word);
                    }

                    //compute data set
                    if(len_words_dataset.containsKey((Integer) word_len)){
                        len_words_dataset.put((Integer) word_len, len_words_dataset.get((Integer) word_len) + 1);
                    } else {
                        len_words_dataset.put((Integer) word_len, 1);
                    }
                }

                //add data into the data pool provided by main thread
                synchronized (Tema2.map_results.get(file_name)){
                    MapDataSet mds = new MapDataSet(len_words_dataset, words_max_len);
                    Tema2.map_results.get(file_name).add(mds);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        Tema2.inQueue.decrementAndGet();
        if(Tema2.inQueue.get() == 0)
            Tema2.tpe_map.shutdown();
    }
}
