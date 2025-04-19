# -*- coding: utf-8 -*-
# 引入依赖包
# pip install alibabacloud_facebody20191230

import os
import io
from urllib.request import urlopen
from alibabacloud_facebody20191230.client import Client
from alibabacloud_facebody20191230.models import SearchFaceAdvanceRequest
from alibabacloud_tea_openapi.models import Config
from alibabacloud_tea_util.models import RuntimeOptions

config = Config(
    # 创建AccessKey ID和AccessKey Secret，请参考https://help.aliyun.com/document_detail/175144.html。
    # 如果您用的是RAM用户的AccessKey，还需要为RAM用户授予权限AliyunVIAPIFullAccess，请参考https://help.aliyun.com/document_detail/145025.html。
    # 从环境变量读取配置的AccessKey ID和AccessKey Secret。运行代码示例前必须先配置环境变量。
    access_key_id=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_ID'),
    access_key_secret=os.environ.get('ALIBABA_CLOUD_ACCESS_KEY_SECRET'),
    # 访问的域名
    endpoint='facebody.cn-shanghai.aliyuncs.com',
    # 访问的域名对应的region
    region_id='cn-shanghai'
)

def alibaba_face():
    search_face_request = SearchFaceAdvanceRequest()
    #场景一：文件在本地
    stream0 = open(r'/tmp/SearchFace.jpg', 'rb')
    search_face_request.image_url_object = stream0

    #场景二：使用任意可访问的url
    #url = 'https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/viapi-3.0domepic/facebody/SearchFace1.png'
    #img = urlopen(url).read()
    #search_face_request.image_url_object = io.BytesIO(img)
    search_face_request.db_name = 'default'
    search_face_request.limit = 5

    runtime_option = RuntimeOptions()
    try:
        # 初始化Client
        client = Client(config)
        response = client.search_face_advance(search_face_request, runtime_option)
        match_list = response.body.to_map()
        # 获取所有 FaceItems 中的 Score
        Scores = [item['Score'] for item in match_list['Data']['MatchList'][0]['FaceItems']]
        # 获取整体匹配度最高的FaceId
        max_score = max(Scores)
        # 取浮点数的前两位作为返回值的结果
        value = round(max_score, 2)
        # 获取整体结果
        print(response.body)
        #print(Scores)
        #print ("匹配度最高的score值",max_score)
        #print(value)
        return value

    except Exception as error:
        # 获取整体报错信息
        print(error)
        # 获取单个字段
        print(error.code)
        # tips: 可通过error.__dict__查看属性名称
        return 0.0
    #关闭流
    #stream0.close()

if __name__ == '__main__':
    alibaba_face()