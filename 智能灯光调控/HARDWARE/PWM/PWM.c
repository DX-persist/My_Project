#include "stm32f10x.h"                  // Device header

/*��ģ���������뿴��Э�Ƽ���ӦP16��Ƶ*/

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//��ǰ�߽���������
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//������GPIOһ������ṹ�壬Ȼ���ʼ������
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //��ʱ��


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������ģʽ������~���Խ�GPIO�ڵĿ���Ȩ����Ƭ������Ҳ���Ƕ�ʱ������ͨ�����������������Ĵ���������GPIO�ڵ�
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //ѡ��PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��������������GPIOA��ʼ��


    //��һ��������ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);/*����ѡ��TIM2������ͨ�ö�ʱ��������ѡ�񣩡�
                                                          *��TIM2��APB1���ߵ����裬�������ﲻ��APB2*/
    //�ڶ�����ѡ��ʱ����Ԫ��ʱ��
    TIM_InternalClockConfig(TIM2);/*��Ϊѡ���ڲ�ʱ�ӵĺ���������TIM2��ʱ����Ԫ�����ڲ�������������
    *������Ϊ��ʱ���ϵ��Ĭ��ʹ���ڲ�ʱ�������������д������ʡ�ԣ�Ϊ�˴��������ԣ�����д��*/

    //������������ʱ����Ԫ

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷ�Ƶ������ѡ��1��Ƶ��Ҳ���ǲ���Ƶ
    //�����˲�����������������Ҫһ������Ƶ�ʲ��ܹ����������Ƶ�ʿ����ڲ�ʱ���Լ�����Ҳ�����ڲ�ʱ�Ӽ�ʱ��Ƶ�ʡ��ɼ���ϵ�������������ѡ�񲻷�Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //����ѡ�����ϼ���

    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; //���ڣ�ARR�Զ���װ����ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1; //PSCԤ��Ƶ����ֵ

    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00; //�ظ���������ֵ���߼���ʱ�����У����ﲻ��Ҫ�ã��͸�0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    //��ʼ������Ƚϵ�Ԫ

    TIM_OCStructInit(&TIM_OCInitStructure);/*�Զ����ṹ�����һ����ʼ��������˵������б�������һ����ʼֵ
    *�����Ҫ��ĳЩ����������ֵ�����������������ó�����ֵ���ɡ���סҪ���ڽṹ�嶨������棬*/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //����PWM1����Ƚ�ģʽ
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //����Ƚϼ��ԣ�ѡ��ߵ�ƽ�ݶ���Ϊѡ���˸ߵ�ƽ����LED��
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ; //���ʹ��
    TIM_OCInitStructure.TIM_Pulse = 0; //��CCR��ֵ
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    //��ʼ������Ƚϵ�Ԫ

    TIM_Cmd(TIM2, ENABLE); //��ʱ����������

}
void PWM1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//��ǰ�߽���������
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//������GPIOһ������ṹ�壬Ȼ���ʼ������
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //��ʱ��


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������ģʽ������~���Խ�GPIO�ڵĿ���Ȩ����Ƭ������Ҳ���Ƕ�ʱ������ͨ�����������������Ĵ���������GPIO�ڵ�
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //ѡ��PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��������������GPIOA��ʼ��


    //��һ��������ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);/*����ѡ��TIM2������ͨ�ö�ʱ��������ѡ�񣩡�
                                                          *��TIM2��APB1���ߵ����裬�������ﲻ��APB2*/
    //�ڶ�����ѡ��ʱ����Ԫ��ʱ��
    TIM_InternalClockConfig(TIM2);/*��Ϊѡ���ڲ�ʱ�ӵĺ���������TIM2��ʱ����Ԫ�����ڲ�������������
    *������Ϊ��ʱ���ϵ��Ĭ��ʹ���ڲ�ʱ�������������д������ʡ�ԣ�Ϊ�˴��������ԣ�����д��*/

    //������������ʱ����Ԫ

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷ�Ƶ������ѡ��1��Ƶ��Ҳ���ǲ���Ƶ
    //�����˲�����������������Ҫһ������Ƶ�ʲ��ܹ����������Ƶ�ʿ����ڲ�ʱ���Լ�����Ҳ�����ڲ�ʱ�Ӽ�ʱ��Ƶ�ʡ��ɼ���ϵ�������������ѡ�񲻷�Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; //����ѡ�����ϼ���

    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; //���ڣ�ARR�Զ���װ����ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1; //PSCԤ��Ƶ����ֵ

    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00; //�ظ���������ֵ���߼���ʱ�����У����ﲻ��Ҫ�ã��͸�0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    //��ʼ������Ƚϵ�Ԫ

    TIM_OCStructInit(&TIM_OCInitStructure);/*�Զ����ṹ�����һ����ʼ��������˵������б�������һ����ʼֵ
    *�����Ҫ��ĳЩ����������ֵ�����������������ó�����ֵ���ɡ���סҪ���ڽṹ�嶨������棬*/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //����PWM1����Ƚ�ģʽ
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //����Ƚϼ��ԣ�ѡ��ߵ�ƽ�ݶ���Ϊѡ���˸ߵ�ƽ����LED��
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ; //���ʹ��
    TIM_OCInitStructure.TIM_Pulse = 0; //��CCR��ֵ
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    //��ʼ������Ƚϵ�Ԫ

    //��������������ʱ��
    TIM_Cmd(TIM2, ENABLE); //��ʱ����������
}

void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare1(TIM2, Compare);
}

void PWM_SetCompare2(uint16_t Compare)//pare1ͬ���ĳ�pare2
{
    TIM_SetCompare2(TIM2, Compare);
}
