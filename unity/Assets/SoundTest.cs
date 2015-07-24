using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class SoundTest : MonoBehaviour
{
    const string BGMInfoOpus = "Opus\nStreaming\nnStereo\n36 kbps VBR\nSize 432746 bytes";
    const string BGMInfoUnity = "Unity Vorbis\nStreaming\nStereo\nQuality 25\nOptimize Sample Rate\nImported Size 441408 bytes";

    const int Flag_Sound = 1;
    const float AttackTime = 0.5f;

    public GameObject se1Prefab_;
    public GameObject bgm1Prefab_;
    public GameObject circlePrefab_;

    public Button toggleBGM_;
    public Text textBGM_;

    private AudioSource se1_;
    private AudioSource bgm1_;

    private System.IntPtr bgm0_ = System.IntPtr.Zero;

    float time_;
    int flag_;
    int index_ = 0;
    bool playBGMOpus_ = true;

    struct Circle
    {
        public float time_;
        public GameObject sprite_;
    };

    Circle[] circles_ = new Circle[2];

    void Start()
    {
        se1_ = (GameObject.Instantiate(se1Prefab_) as GameObject).GetComponent<AudioSource>();
        bgm1_ = (GameObject.Instantiate(bgm1Prefab_) as GameObject).GetComponent<AudioSource>();
        bgm1_.loop = true;

        Vector3 pos = new Vector3(-4.0f, 0.0f, 0.0f);
        for(int i = 0; i < 2; ++i) {
            circles_[i].sprite_ = GameObject.Instantiate(circlePrefab_) as GameObject;
            circles_[i].sprite_.transform.localPosition = pos;
            circles_[i].sprite_.SetActive(false);
            pos.x += 8.0f;
        }

        toggleBGM_.onClick.AddListener(onClickToggleBGM);

        PluginAudio.audioInitialize();
        PluginAudio.loadResourcePackFromAsset(0, "bgm.pak", true);
        PluginAudio.loadResourcePackFromAsset(1, "se.pak", false);

        bgm0_ = PluginAudio.audioCreateUserPlayer(0, 0);
        PluginAudio.audioUserPlayerPlay(bgm0_);
        textBGM_.text = BGMInfoOpus;
    }

    void OnDestroy()
    {
        PluginAudio.audioDestroyUserPlayer(bgm0_);
        bgm0_ = System.IntPtr.Zero;

        PluginAudio.audioTerminate();
    }

    void OnApplicationPause(bool pause)
    {
        PluginAudio.audioPause(pause);
    }

    void LateUpdate()
    {
        PluginAudio.audioProc();
    }

    void onClickToggleBGM()
    {
        if(playBGMOpus_) {
            playBGMOpus_ = false;
            toggleBGM_.GetComponentInChildren<Text>().text = "BGM UNITY NOW";
            PluginAudio.audioUserPlayerPause(bgm0_);
            bgm1_.Play();
            textBGM_.text = BGMInfoUnity;

        } else {
            playBGMOpus_ = true;
            toggleBGM_.GetComponentInChildren<Text>().text = "BGM OPUS NOW";
            PluginAudio.audioUserPlayerPlay(bgm0_);
            bgm1_.Pause();
            textBGM_.text = BGMInfoOpus;
        }
    }

    void Update()
    {
        time_ += Time.deltaTime;

        if(2.0f < time_) {
            time_ -= 2.0f;
            flag_ = 0;
            index_ = (index_ + 1) & 0x01;
        } else if(1.0f < time_) {
            if((flag_ & Flag_Sound) == 0) {
                flag_ |= Flag_Sound;
                show(index_);
                play(index_);
            }
        }

        render();
    }


    void show(int index)
    {
        circles_[index].time_ = AttackTime;
        circles_[index].sprite_.SetActive(true);
    }

    void play(int index)
    {
        switch(index) {
        case 0:
            PluginAudio.audioPlay(1, 0, 1.0f);
            break;

        case 1:
            se1_.PlayOneShot(se1_.clip);
            break;
        }
    }

    void render()
    {
        for(int i = 0; i < circles_.Length; ++i) {
            if(0.0f < circles_[i].time_) {
                circles_[i].time_ -= Time.deltaTime;
                if(circles_[i].time_ < 0.0f) {
                    circles_[i].sprite_.SetActive(false);
                } else {
                    float t = 4.0f*(1.0f - circles_[i].time_ / AttackTime);
                    circles_[i].sprite_.transform.localScale = new Vector3(t, t, 0.0f);
                }
            }
        }
    }
}
