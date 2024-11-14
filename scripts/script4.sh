for folder1 in "f1" "f2" "f3" "f4"; do
    mkdir $folder1
    cd $folder1
    for folder2 in "f1" "f2" "f3" "f4"; do
        mkdir $folder2
        cd $folder2
        for folder3 in "f1" "f2" "f3" "f4"; do
            mkdir $folder3
            cd $folder3
            for file in "f1" "f2" "f3" "f4"; do
                echo $file.$file >$file.$file
            done
            cd ..
        done
        cd ..
    done
    cd ..
done
