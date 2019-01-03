# osproject

## 1.	파일 시스템 설계 

<p align="center"><img src="https://user-images.githubusercontent.com/34092568/50624157-96a23280-0f60-11e9-8b9b-a29e39a0875c.png"></p>
    
    이와 같이 각 노드들은 3개의 링크를 가지고 있다.
    전 노드를 가르키는prev, 다음 노드를가르키는 next, 마지막으로 parent를 가르키는 parent.
    파일과 디렉토리를 별도로 구분하지 않고 하나의 linked list에 모두 연결시켜둔 다음, 
    full path를 통해서 파일인지 디렉토리인지를 구분한다.
    또 하나의 Fidi node는 inode 포인터를 갖으며, 
    이는 파일이나 디렉토리의 권한과 크기 등의 파일 정보를 포함한다.
    위의 설계를 바탕으로 구성한 기본 함수는 다음과 같고 이 자료구조를 토대로 파일 시스템을 구현하였다. 

<p align="center"><img src="https://user-images.githubusercontent.com/34092568/50625459-ad01bb80-0f6b-11e9-9672-aad5ef04d37c.png"></p>

## 2.	FUSE를 이용한 파일시스템 구현

<p align="center"><img src="https://user-images.githubusercontent.com/34092568/50625542-2ef1e480-0f6c-11e9-8ea8-fdddd5210d5d.png" width="500"></p>
<p align="center"><img src="https://user-images.githubusercontent.com/34092568/50625543-2ef1e480-0f6c-11e9-805e-b83b55dbe34b.png"></p>


  ### 2.1	필수 기능 구현
    1)	파일 열기/닫기
    2)	파일 읽기/쓰기
    3)	파일 생성/삭제
    4)	다단계 디렉터리 생성 삭제

  ### 2.2	추가 기능 구현
    1)	파일 이름 재설정
    2)	상황에 맞는 에러 사용

